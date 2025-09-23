/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "../Support/LittleFS_ext.h"
#include "../Support/Time_ext.h"
#include "../Support/NTP.h"


#include "Scheduler.h"

#include "Support/ConstantStrings.h"

AsStringParameter::AsStringParameter(const JsonVariant& json):OnTickExtParameters(0,1)
{
    str = json.as<const char*>();
    //serializeJson(json, jsonStr);  // ArduinoJson overload writes directly into std::string
}

namespace Scheduler
{
    //WEBSERVER_TYPE *webserver = nullptr;

    int FuncCount = 0;
    NameToFunction* nameToFuncList = nullptr;

    TimeAlarmsClass *Scheduler = nullptr;

    // Define a lookup table for day abbreviations to corresponding enum values
    const DayLookupTable dayLookupTable[] = {
        {"mon", timeDayOfWeek_t::dowMonday},
        {"tue", timeDayOfWeek_t::dowTuesday},
        {"wed", timeDayOfWeek_t::dowWednesday},
        {"thu", timeDayOfWeek_t::dowThursday},
        {"fri", timeDayOfWeek_t::dowFriday},
        {"sat", timeDayOfWeek_t::dowSaturday},
        {"sun", timeDayOfWeek_t::dowSunday}
    };

    void HandleAlarms() {
        if (Scheduler != nullptr) Scheduler->delay(0);
        //else // this happens when there are no file loaded, i.e. when a schedule file is not found
        //    Serial.println("Scheduler is null");
    }

    bool LoadJson(String filePath)
    {
        DEBUG_UART.println(F("TAFJ LJ start"));

        if (LittleFS.exists(filePath) == false) { DEBUG_UART.print(F("LJ FNF: ")); DEBUG_UART.println(filePath); return false; }
        
        int fileSize = LittleFS_ext::getFileSize(filePath.c_str());
        if (fileSize == -1) {DEBUG_UART.println(F("LJ FEGFS -1")); return false; }
        
        DEBUG_UART.print(F("file size: ")); DEBUG_UART.println(fileSize);

        //char* buff = (char*)malloc(size+2);
        char *buff = new char[fileSize+2];
        if (buff == nullptr) { DEBUG_UART.println(F("LJ mem err")); return false; }

        if (LittleFS_ext::load_from_file(filePath.c_str(), buff) != LittleFS_ext::FileResult::Success) {
            DEBUG_UART.println(F("LJ LFSe LFFE")); // LoadJson LittleFS_ext::load_from_file error
            //free(buff);
            delete[] buff;
            return false;
        }
        size_t jsonDocBufferSize = (size_t)((float)fileSize * 1.5f);
        DynamicJsonDocument jsonDoc(jsonDocBufferSize);
        DeserializationError jsonStatus = deserializeJson(jsonDoc, buff);
        //DEBUG_UART.print(F("deserialized json document size: ")); DEBUG_UART.println(jsonDoc.memoryUsage());
        if (jsonStatus != DeserializationError::Ok) {
            //free(buff);
            delete[] buff;
            DEBUG_UART.printf("LJ err: "); DEBUG_UART.println((int)jsonStatus.code());
            return false;
        }
        size_t itemCount = jsonDoc.size();
        DEBUG_UART.print(itemCount); DEBUG_UART.println(F(" items"));
        
        // clear both timers and alarms (ALL)
        //for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
        //    Alarm.free(id);
        //} this was the old way before dynamic allocated alarms

        uint8_t nrOfActiveAlarms = 0;
        for (uint8_t i = 0;i<itemCount;i++) {
            if (jsonDoc[i].containsKey("disabled") == false) nrOfActiveAlarms++; 
        }
        if (Scheduler != nullptr) {
            //if (Scheduler->getCurrentNrOfAlarms() < nrOfActiveAlarms) {
                delete Scheduler; // delete old instance
               // Scheduler = new TimeAlarmsClass(nrOfActiveAlarms);
            //}
            //else
            //    Scheduler->clear()
        }
        //else // this happens only the very first time LoadJson is called

        Scheduler = new TimeAlarmsClass(nrOfActiveAlarms);

        // sync time with NTP server
        NTP::NTPConnect();
        tmElements_t now2;
        breakTime(time(nullptr), now2);
        int year = (int)now2.Year + 1970;
        setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);

        for (uint8_t i = 0; i < itemCount; i++) {
            ParseItem(jsonDoc[i]);
        }
        DEBUG_UART.println(F("TAFJ LJ E")); // TimeAlarmsFromJson LoadJson en
        //free(buff);
        delete[] buff;
        return true;
    }

    void ParseItem(const JsonVariant& json)
    {
        //Alarm.alarmOnce();
        //Alarm.alarmRepeat(); 
        //Alarm.timerOnce(); // timer only
        //Alarm.timerRepeat(); // timer only
        //Alarm.triggerOnce(); // dtExplicitAlarm only
        if (json.containsKey("disabled") == true) return;
        //if (json["enabled"] == false) return;

        if (HAL_JSON::ValidateJsonStringField(json, "mode") == false) return;
        const char* mode = HAL_JSON::GetAsConstChar(json, "mode");
        if (CharArray::equalsIgnoreCase(mode, "timer")) // timer repeat only
        {
            JsonBaseVars vars = GetJsonBaseVars(json);
            if (vars.funcName == nullptr) return;
            AlarmID_t id=-1;
            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"]);
                id = Scheduler->timerRepeat(vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                id = Scheduler->timerRepeat(vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }

            //DEBUG_UART.printf("added timer repeat %d:%d:%d  now:%s   Scheduler->read(id):%s   Scheduler->getNextTrigger():%s   Scheduler->getNextTrigger(id):%s\r\n", vars.h, vars.m, vars.s, Time_ext::GetTimeAsString(now()).c_str(), Time_ext::GetTimeAsString(Scheduler->read(id)).c_str() , Time_ext::GetTimeAsString(Scheduler->getNextTrigger()).c_str(), Time_ext::GetTimeAsString(Scheduler->getNextTrigger(id)).c_str());
        
        }
        else if (CharArray::equalsIgnoreCase(mode, "daily"))
        {
            JsonBaseVars vars = GetJsonBaseVars(json);
            if (vars.funcName == nullptr) return;

            AlarmID_t id=-1;
            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"]);
                id = Scheduler->alarmRepeat(vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                id = Scheduler->alarmRepeat(vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }
            
            //DEBUG_UART.printf("added timer daily %d:%d:%d  now:%s   Scheduler->read(id):%s   Scheduler->getNextTrigger():%s   Scheduler->getNextTrigger(id):%s\r\n", vars.h, vars.m, vars.s, Time_ext::GetTimeAsString(now()), Time_ext::GetTimeAsString(Scheduler->read(id)) , Time_ext::GetTimeAsString(Scheduler->getNextTrigger()), Time_ext::GetTimeAsString(Scheduler->getNextTrigger(id)));
        
        }
        else if (CharArray::equalsIgnoreCase(mode, "weekly"))
        {
            JsonBaseVars vars = GetJsonBaseVars(json);
            if (vars.funcName == nullptr) return;

            if (json.containsKey("D") == false) return; // day of week
            timeDayOfWeek_t dow = GetTimerAlarmsDOW(json["D"]);

            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"]);
                Scheduler->alarmRepeat(dow, vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                Scheduler->alarmRepeat(dow, vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }
            //DEBUG_UART.println("added alarm weekly");
        }
        else if (CharArray::equalsIgnoreCase(mode, "explicit"))
        {
            if (json.containsKey("func") == false) return;
            const char* funcName = HAL_JSON::GetAsConstChar(json,"func");
            tmElements_t tm;
            tm.Year = HAL_JSON::GetAsUINT32(json,"Y", 0);
            tm.Month = HAL_JSON::GetAsUINT32(json,"M", 0);
            tm.Day = HAL_JSON::GetAsUINT32(json,"d", 0 );
            tm.Hour = HAL_JSON::GetAsUINT32(json,"h", 0);
            tm.Minute = HAL_JSON::GetAsUINT32(json,"m", 0);
            tm.Second = HAL_JSON::GetAsUINT32(json,"s", 0);
            time_t dateTime = makeTime(tm);
            if(json.containsKey("params")) {
                //JsonVariant jsonVar = json["params"].as<JsonVariant>();
                AsStringParameter *params = new AsStringParameter(json["params"]);
                Scheduler->triggerOnce(dateTime, GetFunctionExt(funcName), params);
            }
            else {
                Scheduler->triggerOnce(dateTime, GetFunction(funcName));
            }
            //DEBUG_UART.println("added alarm explicit");
        }
    }

    OnTick_t GetFunction(const char* name) {
        for (int i = 0; i < FuncCount; i++) {
            if (CharArray::equalsIgnoreCase(nameToFuncList[i].name, name))
                return nameToFuncList[i].onTick;
        }
        return nullptr;
    }
    OnTickExt_t GetFunctionExt(const char* name) {
        for (int i = 0; i < FuncCount; i++) {
            if (CharArray::equalsIgnoreCase(nameToFuncList[i].name, name))
                return nameToFuncList[i].onTickExt;
        }
        return nullptr;
    }

    JsonBaseVars GetJsonBaseVars(const JsonVariant& json)
    {
        if (json.containsKey("func") == false) return {nullptr,0,0,0};

        return {HAL_JSON::GetAsConstChar(json, "func"),
                static_cast<int>(HAL_JSON::GetAsUINT32(json,"h",0)),
                static_cast<int>(HAL_JSON::GetAsUINT32(json,"m",0)),
                static_cast<int>(HAL_JSON::GetAsUINT32(json,"s",0))};
    }

    timeDayOfWeek_t GetTimerAlarmsDOW(std::string sDOW) // short for: short DOW
    {
        /*
        if (sDOW == "mon") return timeDayOfWeek_t::dowMonday;
        else if (sDOW == "tue") return timeDayOfWeek_t::dowTuesday;
        else if (sDOW == "wed") return timeDayOfWeek_t::dowWednesday;
        else if (sDOW == "thu") return timeDayOfWeek_t::dowThursday;
        else if (sDOW == "fri") return timeDayOfWeek_t::dowFriday;
        else if (sDOW == "sat") return timeDayOfWeek_t::dowSaturday;
        else if (sDOW == "sun") return timeDayOfWeek_t::dowSunday;
        else return timeDayOfWeek_t::dowInvalid;
        */
        for (const auto& entry : dayLookupTable) {
            if (sDOW == entry.abbreviation) {
                return entry.dayOfWeek;
            }
        }
        return timeDayOfWeek_t::dowInvalid;
    }

    std::string GetShortFormDowListAsJson()
    {
        int item_Count = sizeof(dayLookupTable) / sizeof(dayLookupTable[0]);
        std::string ret = "[";
        for (int i=0;i<item_Count;i++) {
            ret += "\"" + std::string(dayLookupTable[i].abbreviation) + "\"";
            if (i < (item_Count-1)) ret += ",";
        }
        ret += "]";
        return ret;
    }

    void setup(WEBSERVER_TYPE &srv, NameToFunction* funcDefList, int funcDefListCount) {

        //webserver = &srv;
        FuncCount = funcDefListCount;
        nameToFuncList = funcDefList;
        NTP::NTPConnect();
        tmElements_t now2;
        breakTime(time(nullptr), now2);
        int year = (int)now2.Year + 1970;
        setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);
        

        //DEBUG_UART.printf("nameToFunctionList_Count:%d\r\n", funcDefListCount);
        
        srv.on(SCHEDULER_URL_REFRESH, [](AsyncWebServerRequest *req) {
        if (LoadJson(SCHEDULER_CFG_FILE_PATH))
            req->send(200,CONSTSTR::htmlContentType_TextPlain, F("schedule ld OK"));
        else
            req->send(200,CONSTSTR::htmlContentType_TextPlain, F("schedule ld err"));
        });
        srv.on(SCHEDULER_URL_GET_MAX_NUMBER_OF_ALARMS, [](AsyncWebServerRequest *req) {
            std::string ret = std::to_string(dtNBR_ALARMS);
            req->send(200, CONSTSTR::htmlContentType_TextPlain, ret.c_str());
        });
        srv.on(SCHEDULER_URL_GET_FUNCTION_NAMES, [](AsyncWebServerRequest *req) {

            std::string jsonStr = "{";

            for (int i=0;i<FuncCount;i++) {
                jsonStr += "\""; jsonStr += nameToFuncList[i].name; jsonStr += "\":\""; jsonStr += ((nameToFuncList[i].onTickExt!=nullptr)?"p":""); jsonStr += "\"";

                if (i < (FuncCount-1)) jsonStr += ",";
            }
            jsonStr += "}";
            req->send(200,CONSTSTR::htmlContentType_TextPlain, jsonStr.c_str());
        });
        srv.on(SCHEDULER_URL_GET_SHORT_DOWS, [](AsyncWebServerRequest *req) {
            std::string ret = GetShortFormDowListAsJson();
            req->send(200,CONSTSTR::htmlContentType_TextPlain, ret.c_str());
        });
        srv.on(SCHEDULER_URL_GET_TIME, [](AsyncWebServerRequest *req) {
            std::string nowstr = "{\n\"now\":\"" + Time_ext::GetTimeAsString(now()) + "\",\n";
            nowstr += "\"next trigger\":\"" + Time_ext::GetTimeAsString(Scheduler->getNextTrigger()) + "\"\n}";
            req->send(200,F("text/json"), nowstr.c_str());
        });
        LoadJson(SCHEDULER_CFG_FILE_PATH);
    }
}