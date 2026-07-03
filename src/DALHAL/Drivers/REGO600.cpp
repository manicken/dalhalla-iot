/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

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

#include "REGO600.h"

#if defined(ESP8266) || defined(ESP32)
#include <DALHAL/API/DALHAL_WebSocketAPI.h> // for SendMessage
#else
#include <DALHAL_WebSocketAPI_Windows.h> // for SendMessage
#endif

#include <WString.h>



#include <DALHAL/Support/DALHAL_Logger.h>

#define DRIVERS_REGO600_ERROR_BASE_STR "REGO600 error - "
namespace Drivers {

    REGO600::RegoLookupEntry REGO600::ManualRawEntry;
    bool REGO600::emitErrorsToWebSocket = false;

    bool unpack_nibbles(const uint8_t* src, uint8_t* dst, size_t dstLen) {
        for (size_t i = 0; i < dstLen; ++i)
        {
            uint8_t high = src[i*2], low = src[i*2+1];
            if (high > 0xF || low > 0xF) {
                // corrupted data
                return false;
            }
            dst[i] = (high << 4) + low;
        }
        return true;
    }

    void REGO600::DebugErrorMessage(const char* msg) {
        if (emitErrorsToWebSocket) {
            DALHAL::WebSocketAPI::Broadcast(DRIVERS_REGO600_ERROR_BASE_STR, msg);
        }
        GlobalLogger.Error(F(DRIVERS_REGO600_ERROR_BASE_STR), msg);
    }

    //const size_t CmdVsResponseSizeTable_Count = 12;
    const REGO600::OpCodeInfo OpCodeInfoTable[] = {
        {REGO600::OpCodes::NotSet,              0,  REGO600::RequestType::NotSet},
        {REGO600::OpCodes::ReadFrontPanel,      5,  REGO600::RequestType::Value}, // Read from front panel (keyboard+leds) {reg 09FF+xx}
        {REGO600::OpCodes::WriteFrontPanel,     1,  REGO600::RequestType::WriteConfirm}, // Write to front panel (keyboard+leds) {reg 09FF+xx}
        {REGO600::OpCodes::ReadSystemRegister,  5,  REGO600::RequestType::Value}, // Read from system register (heat curve, temperatures, devices) {reg 1345+xx}
        {REGO600::OpCodes::WriteSystemRegister, 1,  REGO600::RequestType::WriteConfirm}, // Write into system register (heat curve, temperatures, devices) {reg 1345+xx}
        {REGO600::OpCodes::ReadTimerRegisters,  5,  REGO600::RequestType::Value}, // Read from timer registers {reg 1B45+xx}
        {REGO600::OpCodes::WriteTimerRegisters, 1,  REGO600::RequestType::WriteConfirm}, // Write into timer registers {reg 1B45+xx}
        {REGO600::OpCodes::ReadRegister_1B61,   5,  REGO600::RequestType::Value}, // Read from register 1B61 {reg 1B61+xx}
        {REGO600::OpCodes::WriteRegister_1B61,  1,  REGO600::RequestType::WriteConfirm}, // Write into register 1B61 {1B61+xx}
        {REGO600::OpCodes::ReadDisplay,         42, REGO600::RequestType::Text}, // Read from display {0AC7+15h*xx}
        {REGO600::OpCodes::ReadLastError,       42, REGO600::RequestType::ErrorLogItem}, // Read last error line [4100/00]
        {REGO600::OpCodes::ReadPrevError,       42, REGO600::RequestType::ErrorLogItem}, // Read previous error line (prev from last reading) [4100/01]
        {REGO600::OpCodes::ReadRegoVersion,     5,  REGO600::RequestType::Value} // Read rego version {constant 0258 = 600 ?Rego 600?}
    };
    const REGO600::OpCodeInfo& REGO600::getCmdInfo(uint8_t opcode) {

        for (const auto& entry : OpCodeInfoTable) {
            if (static_cast<uint8_t>(entry.opcode) == opcode) {
                return entry;
            }
        }
        return OpCodeInfoTable[0]; // Not found
    }
#if defined(ESP8266) || defined(AVR)
#define STRING_PLACEMENT_POLICY PROGMEM
#else
#define STRING_PLACEMENT_POLICY
#endif

    static const char type_str_GT1[] STRING_PLACEMENT_POLICY = "GT1";
    static const char type_str_GT2[] STRING_PLACEMENT_POLICY = "GT2";
    static const char type_str_GT3[] STRING_PLACEMENT_POLICY = "GT3";
    static const char type_str_GT4[] STRING_PLACEMENT_POLICY = "GT4";
    static const char type_str_GT5[] STRING_PLACEMENT_POLICY = "GT5";
    static const char type_str_GT6[] STRING_PLACEMENT_POLICY = "GT6";
    static const char type_str_GT8[] STRING_PLACEMENT_POLICY = "GT8";
    static const char type_str_GT9[] STRING_PLACEMENT_POLICY = "GT9";
    static const char type_str_GT10[] STRING_PLACEMENT_POLICY = "GT10";
    static const char type_str_GT11[] STRING_PLACEMENT_POLICY = "GT11";
    static const char type_str_GT3x[] STRING_PLACEMENT_POLICY = "GT3x";

    static const char type_str_P3[] STRING_PLACEMENT_POLICY = "P3";
    static const char type_str_COMP[] STRING_PLACEMENT_POLICY = "COMP";
    static const char type_str_EL3[] STRING_PLACEMENT_POLICY = "EL3";
    static const char type_str_EL6[] STRING_PLACEMENT_POLICY = "EL6";
    static const char type_str_P1[] STRING_PLACEMENT_POLICY = "P1";
    static const char type_str_P2[] STRING_PLACEMENT_POLICY = "P2";
    static const char type_str_VXV[] STRING_PLACEMENT_POLICY = "VXV";
    static const char type_str_ALARM[] STRING_PLACEMENT_POLICY = "ALARM";


    constexpr REGO600::RegoLookupEntry SystemRegisterTable[] = {
        // Namn,   Adr,    Opcode,                Min,    Max,   Signed, Multiplier
        // temperature sensor registers
        {type_str_GT1,  0x0209,  {.s16 = -500},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Radiator return (GT1)
        {type_str_GT2,  0x020A,  {.s16 = -500},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Outdoor (GT2)
        {type_str_GT3,  0x020B,  {.s16 = 10},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Hot water (GT3)
        {type_str_GT4,  0x020C,  {.s16 = 10},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Forward (GT4)
        {type_str_GT5,  0x020D,  {.s16 = 10},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Room (GT5)
        {type_str_GT6,  0x020E,  {.s16 = 10},  {.s16 = 1500},  REGO600::ValueType::Float,  0.1f}, // Compressor (GT6) 
        {type_str_GT8,  0x020F,  {.s16 = 10},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Heat fluid out (GT8)
        {type_str_GT9,  0x0210,  {.s16 = 10},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Heat fluid in (GT9)
        {type_str_GT10, 0x0211,  {.s16 = -500},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Cold fluid in (GT10)
        {type_str_GT11, 0x0212,  {.s16 = -500},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // Cold fluid out (GT11)
        {type_str_GT3x, 0x0213,  {.s16 = 10},  {.s16 = 1200},  REGO600::ValueType::Float,  0.1f}, // External hot water (GT3x)
        // state registers
        {type_str_P3,    0x01FD, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Ground loop pump [P3]
        {type_str_COMP,  0x01FE, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Compresor
        {type_str_EL3,   0x01FF, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Additional heat 3kW
        {type_str_EL6,   0x0200, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Additional heat 6kW
        {type_str_P1,    0x0203, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Radiator pump [P1]
        {type_str_P2,    0x0204, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Heat carrier pump [P2]
        {type_str_VXV,   0x0205, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Tree-way valve [VXV]
        {type_str_ALARM, 0x0206, {.u16 = 0}, {.u16 = 1},  REGO600::ValueType::Bool, 1.0f}, // Alarm
    };
    constexpr size_t SystemRegisterTableSize = sizeof(SystemRegisterTable)/sizeof(SystemRegisterTable[0]);

    const REGO600::RegoLookupEntry* REGO600::SystemRegisterTableLockup(const char* name) {
        DALHAL::ZeroCopyString zcName(name);
        for (size_t i = 0; i < SystemRegisterTableSize; i++) {
#if defined(ESP8266) || defined(AVR)
            if (zcName.EqualsIC_P(SystemRegisterTable[i].name)) {
#else
            if (zcName.EqualsIC(SystemRegisterTable[i].name)) {
#endif
                return &SystemRegisterTable[i];
            }
        }
        return nullptr;
    }

    bool REGO600::SystemRegisterTable_ItemExists(void* ctx, const char* name) { // ctx not used here as this is a static table
        return SystemRegisterTableLockup(name) != nullptr;
    }
    void REGO600::SystemRegisterTable_GetAllNamesAsJsonStringArray(void* ctx, DALHAL::StringBuilderStreamer& sbs) { // ctx not used here as this is a static table
        sbs.write_json_array_begin();
        for (size_t i = 0; i < SystemRegisterTableSize; i++) {
            if (i > 0) {
                sbs.write_json_value_separator();
            }
            sbs.write_jsonQuoted(SystemRegisterTable[i].name);
        }
        sbs.write_json_array_end();
    }

    // Constructor for linked values here the type is allways Value
    REGO600::Request::Request(const OpCodeInfo& _info, const RegoLookupEntry& _def, DALHAL::HALValue& externalValue) 
        : info(_info), 
        def(_def)
    {
        response.value = &externalValue;
    }

    REGO600::Request::Request(const OpCodeInfo& _info, const RegoLookupEntry& _def) 
        : info(_info), 
          def(_def)
    {
        if (info.type == RequestType::Text) {
            response.text = new char[21](); // 20 characters + null terminator
        }
        else if (info.type == RequestType::Value) {
            response.value = new DALHAL::HALValue();
        }
        else if (info.type == RequestType::ErrorLogItem) {
            response.text = new char[20](); // 3 digit error code + space + 6 char date + space + 8 char time + null terminator
        }
    }
    bool REGO600::Request::ValidateAndSetFromBuffer(uint8_t* buff) {
        if (info.type == RequestType::Value && response.value) {
            // 1. Extract the 16-bit raw value (7-bit packing)
            uint16_t rawValue = (buff[1] << 14) + (buff[2] << 7) + buff[3];
            
            // 2. Perform Type-Aware Validation
            switch (def.valueType) {
                case ValueType::Signed: {
                    int16_t sVal = static_cast<int16_t>(rawValue);
                    if (sVal >= def.minVal.s16 && sVal <= def.maxVal.s16) {
                        response.value->set((int32_t)(sVal*def.multiplier));
                    } else {
                        DebugErrorMessage(String(F("skipped Signed value because out of range")).c_str());
                        return false;
                    }
                    break;
                }
                case ValueType::Bool:
                    if (rawValue == 0 || rawValue == 1) {
                        response.value->set((uint32_t)rawValue);
                    } else {
                        DebugErrorMessage(String(F("skipped Bool value because out of range")).c_str());
                        return false;
                    }
                    break;
                case ValueType::Unsigned:
                    if (rawValue >= def.minVal.u16 && rawValue <= def.maxVal.u16) {
                        response.value->set((uint32_t)(rawValue*def.multiplier));
                    } else {
                        DebugErrorMessage(String(F("skipped Unsigned value because out of range")).c_str());
                        return false;
                    }
                    break;
                case ValueType::Float: { 
                    int16_t sVal = static_cast<int16_t>(rawValue);
                    if (sVal >= def.minVal.s16 && sVal <= def.maxVal.s16) {
                        response.value->set(sVal*def.multiplier);
                    } else {
                        DebugErrorMessage(String(F("skipped Float value because out of range")).c_str());
                        return false;
                    }
                    break;
                }
                default: 
                    break; // to avoid making infinite request loop on unset type entities
            }

        } else if (info.type == RequestType::Text) {
            uint8_t tmp[20];
            
            if (!unpack_nibbles(&buff[1], tmp, 20)) {
                DebugErrorMessage(String(F("LCD data corruption detected - discarding frame - ")).c_str());
                return false;
            }

            memcpy(response.text, tmp, 20);
            response.text[20] = '\0';
        } else if (info.type == RequestType::ErrorLogItem) {
            
            uint32_t code = buff[1]*16 + buff[2];
            response.text[0] = (code / 100)+0x30;
            code %= 100;
            response.text[1] = (code / 10)+0x30;
            code %= 10;
            response.text[2] = (code)+0x30;
            response.text[3] = 0x20; // space
            for (int bi=3,ti=4;ti<20;bi+=2,ti++) {
                response.text[ti] = buff[bi]*16 + buff[bi+1];
            }
            
        } // there are currently no more types right now

        return true;
    }
    REGO600::Request::~Request() {
        // delete owning
        if ((info.type == RequestType::Text || info.type == RequestType::ErrorLogItem) && response.text != nullptr) {
            delete[] response.text;
        }
    }
    std::string REGO600::Request::ToString() {
        std::string str = "";
        str += "info.op: " + std::to_string((int)info.opcode);
        str += ", info.size: " + std::to_string((int)info.size);
        str += ", def.name: " + std::string(def.name);
        str += ", def.address: " + std::to_string(def.address);
        if (def.valueType == ValueType::Signed || def.valueType == ValueType::Float) {
            str += ", def.minVal.s16: " + std::to_string(def.minVal.s16);
            str += ", def.maxVal.s16: " + std::to_string(def.maxVal.s16);
        } else {
            str += ", def.minVal.u16: " + std::to_string(def.minVal.u16);
            str += ", def.maxVal.u16: " + std::to_string(def.maxVal.u16);
        }

        return str;
    }

    //  ██████  ███████  ██████   ██████   ██████   ██████   ██████  
    //  ██   ██ ██      ██       ██    ██ ██       ██  ████ ██  ████ 
    //  ██████  █████   ██   ███ ██    ██ ███████  ██ ██ ██ ██ ██ ██ 
    //  ██   ██ ██      ██    ██ ██    ██ ██    ██ ████  ██ ████  ██ 
    //  ██   ██ ███████  ██████   ██████   ██████   ██████   ██████  

    REGO600::REGO600(int8_t rxPin, int8_t txPin, Request** refreshLoopList, int refreshLoopCount, uint32_t refreshTimeMs, unsigned long requestDelayMs) : 
        refreshLoopList(refreshLoopList), 
        refreshLoopCount(refreshLoopCount),
        refreshTimeMs(refreshTimeMs)
    {

        uint32_t minRefreshTimeMs = refreshLoopCount * REGO600_DRIVER_READ_REGISTER_TIME_MS_ON_STATE;
        // subtract min from wanted to get total refresh time
        this->refreshTimeMs -= minRefreshTimeMs; 
        // ensure it's at least minRefreshTimeMs
        if (this->refreshTimeMs < minRefreshTimeMs) this->refreshTimeMs = minRefreshTimeMs; 
        
        if (requestDelayMs < 10) { // minimum safe
            requestDelayMs = 10;
        }
        pendingRequestDelayMs = requestDelayMs;

        uartTxBuffer[0] = 0x81; // constant
        #if defined(ESP32) && !defined(seeed_xiao_esp32c3)
        REGO600_UART_TO_USE.begin(19200, SERIAL_8N1, rxPin, txPin); // Set correct RX/TX pins for UART
    #elif defined(ESP8266)
        REGO600_UART_TO_USE.begin(19200, SERIAL_8N1); // note on esp8266 pins are not reconfigurable
    #elif defined(seeed_xiao_esp32c3)
        REGO600_UART_TO_USE.begin(19200);
    #endif
    }

    REGO600::~REGO600() {
        if (readLCD_Text != nullptr)
            delete[] readLCD_Text;
        REGO600_UART_TO_USE.flush();
        REGO600_UART_TO_USE.end(); // free up the UART hardware and release TX/RX pins for other use
        // special note here
        // Request* const* refreshLoopList; is deleted outside of this class
    }

    void REGO600::begin() {
        FlushCleanUARTRxBuffer(REGO600_UART_TO_USE, 260); // failsafe and also a quick way to determine if there are any hardware problems (it gets logged to GlobalLogger)
        RefreshLoop_Restart();
    }

    void REGO600::ManualRequest_Schedule(RequestMode reqMode) {
        manualRequest_Mode = reqMode;
        if (requestInProgress == false) {
            ManualRequest_PrepareAndSend(); // this will start send the request
        }
        else {
            manualRequest_Pending = true;
        }
    }
    bool REGO600::ManualRequest_PrepareAndSend() {
        if (manualRequest_Mode == RequestMode::Lcd) {
            uartTxBuffer[1] = static_cast<uint8_t>(OpCodes::ReadDisplay);
            readLCD_RowIndex = 0;
            currentExpectedRxLength = 42;
            SetRequestAddr(0x00);
            uartTxBuffer[8] = 0x00;
            if (readLCD_Text == nullptr) // initialize it, if this is the first use
                readLCD_Text = new char[20*4+1]();
            

        } else if (manualRequest_Mode == RequestMode::FrontPanelLeds) {
            readFrontPanelLeds_Data = 0x00;
            uartTxBuffer[1] = static_cast<uint8_t>(OpCodes::ReadFrontPanel);
            currentExpectedRxLength = 5;
            readFrontPanelLedsIndex = 0;
            SetRequestAddr(0x12);
            CalcAndSetTxChecksum();
            
        } else if (manualRequest_Mode == RequestMode::OneTime && manualRequest != nullptr) {
            uartTxBuffer[1] = (uint8_t)manualRequest->info.opcode;
            currentExpectedRxLength = manualRequest->info.size;
            SetRequestAddr(manualRequest->def.address);
            CalcAndSetTxChecksum();
            
        } else {
            return false;
        }
        mode = manualRequest_Mode;
        //const CmdVsResponseSize* info = getCmdInfo(uartTxBuffer[1]);
        ScheduleNextRequest();
        //SendRequestFrameAndResetRx(); // ManualRequest_PrepareAndSend
        return true;
    }

    bool REGO600::OneTimeRequest(std::unique_ptr<Request> req, RequestCallback cb, void* cb_ctx) {
        if (cb == nullptr) {
            DebugErrorMessage(String(F("OneTimeRequest - callback cannot be nullptr")).c_str());
            return false; // no point if cb for some reason is nullptr
        }
        if (req->info.type != RequestType::WriteConfirm && cb == nullptr) {
            return false; // no point if cb for some reason is nullptr
        }
        if (mode != RequestMode::RefreshLoop) { 
            DebugErrorMessage(String(F("OTReq - manual request allready in progress")).c_str());
            return false;
        }
        manualRequest_Callback = cb;
        manualRequest_Context = cb_ctx;
        manualRequest = std::move(req); // could also do req.release() to return the raw ptr, but then the raw ptr needs to be deleted when used

        ManualRequest_Schedule(RequestMode::OneTime);
        return true;
    }

    bool REGO600::RequestWholeLCD(RequestCallback cb, void* cb_ctx) {
        if (cb == nullptr) {
            DebugErrorMessage(String(F("ReqLCD - callback cannot be nullptr")).c_str());
            return false; // no point if cb for some reason is nullptr
        }
        if (mode != RequestMode::RefreshLoop) { 
            DebugErrorMessage(String(F("ReqLCD - manual request allready in progress")).c_str());
            return false;
        }
        manualRequest_Callback = cb;
        manualRequest_Context = cb_ctx;
        ManualRequest_Schedule(RequestMode::Lcd);
        return true;
    }
    bool REGO600::RequestFrontPanelLeds(RequestCallback cb, void* cb_ctx) {
        if (cb == nullptr) {
            DebugErrorMessage(String(F("ReqFrontPanelLeds - callback cannot be nullptr")).c_str());
            return false; // no point if cb for some reason is nullptr
        }
        if (mode != RequestMode::RefreshLoop) { 
            DebugErrorMessage(String(F("ReqFrontPanelLeds - manual request allready in progress")).c_str());
            return false;
        }
        manualRequest_Callback = cb;
        manualRequest_Context = cb_ctx;
        ManualRequest_Schedule(RequestMode::FrontPanelLeds);
        return true;
    }

    void REGO600::RefreshLoop_SendCurrent() {
        Request* req = refreshLoopList[refreshLoopIndex];

        uartTxBuffer[1] = (uint8_t)req->info.opcode;
        SetRequestAddr(req->def.address);
        CalcAndSetTxChecksum();
       //const CmdVsResponseSize* info = getCmdInfo(refreshLoopList[refreshLoopIndex]->opcode);
        currentExpectedRxLength = req->info.size;
        ScheduleNextRequest();
        //SendRequestFrameAndResetRx(); // RefreshLoop_SendCurrent
    }

    void REGO600::RefreshLoop_Restart() {
        lastUpdateMs = millis();
        refreshLoopIndex = 0;
        RefreshLoop_SendCurrent();   
    }

    void REGO600::RefreshLoop_Continue() {
        if (refreshLoopIndex < (refreshLoopCount-1)) {
            refreshLoopIndex++;
            RefreshLoop_SendCurrent();
        } else {
            refreshLoopDone = true;
            // one loop done
            // exec some cb here, or set some flags
            
            requestInProgress = false; // wait until refresh time 
        }
    }
    bool REGO600::RefreshLoopDone() {
        if (refreshLoopDone == false) {
            return false;
        }
        refreshLoopDone = false;
        return true;
    }

    void REGO600::RxDone_RefreshLoop() {
        if (refreshLoopList[refreshLoopIndex]->ValidateAndSetFromBuffer(uartRxBuffer) == false) {
            DebugErrorMessage(String(F("refreshLoopList RX - invalid value ")).c_str());
            SendRequestFrameAndResetRx(); // retry @ RefreshLoop value error
            return;
        }
        
        if (manualRequest_Pending) {
            manualRequest_Pending = false;
            if (ManualRequest_PrepareAndSend() == true) {
                return;
            }
        }
        RefreshLoop_Continue();
    }
    void REGO600::RxDone_LCD() {
        uint8_t tmp[20];
        if (!unpack_nibbles(&uartRxBuffer[1], tmp, 20)) {
            DebugErrorMessage(String(F("LCD data corruption detected - discarding frame - ")).c_str());
            SendRequestFrameAndResetRx(); // retry @ LCD data error
            return;
        }
        
        memcpy(&readLCD_Text[readLCD_RowIndex*20], tmp, 20);
        
        if (readLCD_RowIndex == 3) { // this was the last row
            
            // execute a callback here
            if (manualRequest_Callback != nullptr) {
                manualRequest_Callback(manualRequest_Context, readLCD_Text, manualRequest_Mode);
            } else {
                DebugErrorMessage(String(F("LCD - mReqCB not set")).c_str());
            }

            mode = RequestMode::RefreshLoop;
            manualRequest_Mode = RequestMode::RefreshLoop;
            RefreshLoop_Continue();
        } else {
            readLCD_RowIndex++;
            uartTxBuffer[4] = readLCD_RowIndex;
            uartTxBuffer[8] = readLCD_RowIndex;
            ScheduleNextRequest();
            //SendRequestFrameAndResetRx(); // LCD continue
        }
    }
    void REGO600::RxDone_FrontPanelLeds() {
        if (uartRxBuffer[3] == 0x01) { 
            readFrontPanelLeds_Data |= 0x01;
        }
        if (readFrontPanelLedsIndex != 4) {
            readFrontPanelLedsIndex++;
            readFrontPanelLeds_Data <<= 1; // shift data to the right
            uartTxBuffer[4] = readFrontPanelLedsIndex + 0x12;
            uartTxBuffer[8] = readFrontPanelLedsIndex + 0x12;
            ScheduleNextRequest();
            //SendRequestFrameAndResetRx(); // FrontPanelLeds continue
        } else {
            
            if (manualRequest_Callback != nullptr) {
                manualRequest_Callback(manualRequest_Context, &readFrontPanelLeds_Data, manualRequest_Mode);
            } else {
                DebugErrorMessage(String(F("FP - mReqCB not set")).c_str());
            }
            mode = RequestMode::RefreshLoop;
            manualRequest_Mode = RequestMode::RefreshLoop;
            RefreshLoop_Continue();
        }
    }
    void REGO600::RxDone_OneTime() {
        if (manualRequest_Callback != nullptr) {
            // only set here, there is no point setting the data if there are no receiver
            // actually making the request in the first place when
            // the callback is not set is actually pointless
            if (manualRequest->ValidateAndSetFromBuffer(uartRxBuffer) == false) {
                // try the request again
                DebugErrorMessage(String(F("manualReq RX - ValidateAndSetFromBuffer error")).c_str());
                SendRequestFrameAndResetRx(); // retry @ OneTime value error
                return;
            }
            //Request* request = manuallyRequest.release();
            manualRequest_Callback(manualRequest_Context, manualRequest.get(), manualRequest_Mode);
            //delete request;
        }
        else {
            //manuallyRequest.reset(); // free the current data
            DebugErrorMessage(String(F("OT - mReqCB not set")).c_str());
        }
        manualRequest.reset(); // free the current data
        mode = RequestMode::RefreshLoop;
        manualRequest_Mode = RequestMode::RefreshLoop;
        RefreshLoop_Continue();
    }

    void REGO600::SendRequestFrameAndResetRx() {
        lastRequestMs = millis();
        uartRxBufferIndex = 0;
        requestInProgress = true;
        REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
    }

    void REGO600::SetRequestAddr(uint16_t address) {
        uartTxBuffer[2] = (address >> 14) & 0x7F;
        uartTxBuffer[3] = (address >> 7) & 0x7F;
        uartTxBuffer[4] = address & 0x7F;
        uartTxBuffer[5] = 0x00; // allways zero unless set
        uartTxBuffer[6] = 0x00; // allways zero unless set
        uartTxBuffer[7] = 0x00; // allways zero unless set
    }

    void REGO600::SetRequestData(uint16_t data) {
        uartTxBuffer[5] = (data >> 14) & 0x7F;
        uartTxBuffer[6] = (data >> 7) & 0x7F;
        uartTxBuffer[7] = data & 0x7F;
    }

    void REGO600::CalcAndSetTxChecksum() {
        uint8_t chksum = 0;
        for (int i=2;i<8;i++) {
            chksum ^= uartTxBuffer[i];
        }
        uartTxBuffer[8] = chksum;
    }

    bool REGO600::CalcAndCompareRxDataChecksum() {
        if (currentExpectedRxLength < 3) {
            // no checksum byte to validate for e.g. 1-byte WriteConfirm acks
            return true;
        }
        uint8_t chksum = 0;

        const uint32_t checksumIndex = currentExpectedRxLength - 1;
        const uint32_t lastDataIndex = checksumIndex - 1;

        for (uint32_t i = 1; i <= lastDataIndex; ++i)
        {
            chksum ^= uartRxBuffer[i];
        }

        return chksum == uartRxBuffer[checksumIndex];
    }

    uint16_t REGO600::GetValueFromUartRxBuff() {
        return (uartRxBuffer[1] << 14) + (uartRxBuffer[2] << 7) + uartRxBuffer[3];
    }

    void REGO600::FlushCleanUARTRxBuffer(REGO600_UART_TYPE& uart, size_t maxDrains) {
        size_t count = 0;
        while (uart.available() && count++ < maxDrains) {
            uart.read();
        }
    }
    void REGO600::ScheduleNextRequest() {
        pendingRequestLastTime = millis();
        pendingRequest = true;
        requestInProgress = true;
    }

    void REGO600::ParseCurrentRxPacket() {
        FlushCleanUARTRxBuffer(REGO600_UART_TO_USE); // allways flush remaining garbage if any

        if (uartRxBuffer[0] == 0x40) {
            DebugErrorMessage(String(F("REGO600 RX failure: device returned error 0x40 in ACK, restarting request")).c_str());
            SendRequestFrameAndResetRx(); // retry @ start byte error
            return;
        }
        if (uartRxBuffer[0] != 0x01) {
            DebugErrorMessage((String(F("RX done - corrupted frame - start byte mismatch:")) + String(uartRxBuffer[0], 16)).c_str());
            SendRequestFrameAndResetRx(); // retry @ start byte error
            return;
        }

        if (CalcAndCompareRxDataChecksum() == false) {
            // try the request again
            DebugErrorMessage(String(F("manualReq RX - Checksum error")).c_str());
            SendRequestFrameAndResetRx(); // retry @ checksum error
            return;
        }
        // RX is done
        if (mode == RequestMode::RefreshLoop) {
            RxDone_RefreshLoop();           
        } else if (mode == RequestMode::Lcd) {
            RxDone_LCD();
        } else if (mode == RequestMode::FrontPanelLeds) {
            RxDone_FrontPanelLeds();
        } else if (mode == RequestMode::OneTime) {
            RxDone_OneTime();
        }
        lastRequestMs = millis();
    }

    #define REGO600_UART_RX_MAX_FAILSAFECOUNT 100
    void REGO600::loop() {

        { // making now into a separate scope for nicer code
            unsigned long now = millis();
            if (pendingRequest && (now - pendingRequestLastTime) > pendingRequestDelayMs) {
                pendingRequest = false;
                SendRequestFrameAndResetRx();
            }
        }

        if (requestInProgress == false) { 
            //  here we just take care of any glitches and receive garbage data if any
            FlushCleanUARTRxBuffer(REGO600_UART_TO_USE);
            //if (mode != RequestMode::RefreshLoop) { return; }
            if (refreshLoopList == nullptr) { return; }
            unsigned long now = millis();
            if (now - lastUpdateMs >= refreshTimeMs) {
                //DALHAL::WebSocketAPI::Broadcast("RefreshLoop_Restart"); // just to see that this worked
                RefreshLoop_Restart(); // this will also take care of updating lastUpdateMs, it also sets requestInProgress to true
            }
            return; // usually dont expect a response directly
        }
        

        uint32_t failsafeReadCount = 0;
        while (REGO600_UART_TO_USE.available() && failsafeReadCount++ < REGO600_UART_RX_MAX_FAILSAFECOUNT) {
            lastRequestMs = millis(); // update on every rx
            if (uartRxBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
                uartRxBuffer[uartRxBufferIndex++] = REGO600_UART_TO_USE.read();
                if (uartRxBufferIndex == currentExpectedRxLength) {
                    ParseCurrentRxPacket();
                    return;                    
                }
            } else {
                requestInProgress = false; // to make the remaining data reads faster, if any 
                mode = RequestMode::RefreshLoop;
                FlushCleanUARTRxBuffer(REGO600_UART_TO_USE);
                DebugErrorMessage(String(F("uartRxBuffer full")).c_str());
            }
            lastRequestMs = millis();
        }
        // Timeout check after possible any rx
        { // making now into a separate scope for nicer code
            unsigned long now = millis();
            if (pendingRequest == false && (now - lastRequestMs) >= requestTimeoutMs) {

                DebugErrorMessage((String(F("REGO600-request-timeout @ index=")) + String(uartRxBufferIndex) + String(F(", expected length=")) + String(currentExpectedRxLength)).c_str());
                //GlobalLogger.Error(F("REGO600-request-timeout")); // only log to logger to not fill serial/websocket with stuff
                //DALHAL::WebSocketAPI::Broadcast("REGO600-request-timeout"); // not needed anymore as logging to GlobalLogger automatically do it on websocket as well
                
                FlushCleanUARTRxBuffer(REGO600_UART_TO_USE);
                SendRequestFrameAndResetRx(); // retry @ timeout
            }
            if (failsafeReadCount == REGO600_UART_RX_MAX_FAILSAFECOUNT) {
                DebugErrorMessage(String(F("read failsafe overflow")).c_str());
            }
        }
    }
}