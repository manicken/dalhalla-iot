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
#include <DALHAL/API/WebSocket/DALHAL_WebSocketAPI.h> // for SendMessage
#else
#include <DALHAL_WebSocketAPI_Windows.h> // for SendMessage
#endif

#include <WString.h>



#include <DALHAL/Support/DALHAL_Logger.h>


namespace Drivers {

    namespace REGO600 {

        REGO600::RegoLookupEntry REGO600Driver::ManualRawEntry;
        DALHAL::HALValue REGO600Driver::ManualRequestValue;

        // theese are the only available op-codes (verified by rom-dump)
        const REGO600::OpCodeInfo OpCodeInfoTable[] = {
            {REGO600::CommandID::NotSet,              0xFF, 0,  REGO600::ResponseType::NotSet},
            {REGO600::CommandID::ReadFrontPanel,      0x00, 5,  REGO600::ResponseType::Value}, // Read from front panel (keyboard+leds) {reg 0x09A6+xx*2}
            {REGO600::CommandID::WriteFrontPanel,     0x01, 1,  REGO600::ResponseType::WriteConfirm}, // Write to front panel (keyboard+leds) {reg 09A6+xx*2}
            {REGO600::CommandID::ReadSystemRegister,  0x02, 5,  REGO600::ResponseType::Value}, // Read from system register (heat curve, temperatures, devices) {reg 0x12EC+xx*2}
            {REGO600::CommandID::WriteSystemRegister, 0x03, 1,  REGO600::ResponseType::WriteConfirm}, // Write into system register (heat curve, temperatures, devices) {reg 0x12EC+xx*2}
            {REGO600::CommandID::ReadTimerRegisters,  0x04, 5,  REGO600::ResponseType::Value}, // Read from timer registers {reg 0x1AEC+xx*2}
            {REGO600::CommandID::WriteTimerRegisters, 0x05, 1,  REGO600::ResponseType::WriteConfirm}, // Write into timer registers {reg 0x1AEC+xx*2}
            {REGO600::CommandID::ReadRegister_1B08,   0x06, 5,  REGO600::ResponseType::Value}, // Read from register 1b08 {reg 1B08+xx*2}
            {REGO600::CommandID::WriteRegister_1B08,  0x07, 1,  REGO600::ResponseType::WriteConfirm}, // Write into register 1b08 {1B08+xx*2}
            {REGO600::CommandID::ReadDisplay,         0x20, 42, REGO600::ResponseType::Text}, // Read from display {0x0A6E+21*xx}
            {REGO600::CommandID::RamDump,             0x20, 42, REGO600::ResponseType::RawBytes}, // Read from raw ram {0x0A6E+21*xx}
            {REGO600::CommandID::ReadLastError,       0x40, 42, REGO600::ResponseType::ErrorLogItem}, // Read last error line [4100/00] (read from external flash)
            {REGO600::CommandID::ReadPrevError,       0x42, 42, REGO600::ResponseType::ErrorLogItem}, // Read previous error line (prev from last reading) [4100/01](read from external flash)
            {REGO600::CommandID::ReadRegoVersion,     0x7F, 5,  REGO600::ResponseType::Value}, // Read rego version {constant 0258 = 600 ?Rego 600?}
        };
        const OpCodeInfo& REGO600Driver::getCmdInfo(CommandID cmdId) {

            for (const auto& entry : OpCodeInfoTable) {
                if (entry.id == cmdId) {
                    return entry;
                }
            }
            return OpCodeInfoTable[0]; // 
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

        const RegoLookupEntry* REGO600Driver::SystemRegisterTableLockup(const char* name) {
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

        bool REGO600Driver::SystemRegisterTable_ItemExists(void* ctx, const char* name) { // ctx not used here as this is a static table
            return SystemRegisterTableLockup(name) != nullptr;
        }
        void REGO600Driver::SystemRegisterTable_GetAllNamesAsJsonStringArray(void* ctx, DALHAL::StringBuilderStreamer& sbs) { // ctx not used here as this is a static table
            sbs.write_json_array_begin();
            for (size_t i = 0; i < SystemRegisterTableSize; i++) {
                if (i > 0) {
                    sbs.write_json_value_separator();
                }
                sbs.write_jsonQuoted(SystemRegisterTable[i].name);
            }
            sbs.write_json_array_end();
        }

        

        //  ██████  ███████  ██████   ██████   ██████   ██████   ██████  
        //  ██   ██ ██      ██       ██    ██ ██       ██  ████ ██  ████ 
        //  ██████  █████   ██   ███ ██    ██ ███████  ██ ██ ██ ██ ██ ██ 
        //  ██   ██ ██      ██    ██ ██    ██ ██    ██ ████  ██ ████  ██ 
        //  ██   ██ ███████  ██████   ██████   ██████   ██████   ██████  

        REGO600Driver::REGO600Driver(int8_t rxPin, int8_t txPin, Request** refreshLoopList, int refreshLoopCount, uint32_t refreshTimeMs, unsigned long requestDelayMs) : 
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

        REGO600Driver::~REGO600Driver() {
            if (readLCD_Text != nullptr)
                delete[] readLCD_Text;
            if (ramDumpBytes != nullptr)
                delete[] ramDumpBytes;
            REGO600_UART_TO_USE.flush();
            REGO600_UART_TO_USE.end(); // free up the UART hardware and release TX/RX pins for other use
            // special note here
            // Request* const* refreshLoopList; is deleted outside of this class
        }

        void REGO600Driver::begin() {
            FlushCleanUARTRxBuffer(REGO600_UART_TO_USE, 260); // failsafe and also a quick way to determine if there are any hardware problems (it gets logged to GlobalLogger)
            RefreshLoop_Restart();
        }

        void REGO600Driver::ManualRequest_Schedule(RequestMode reqMode) {
            manualRequest_Mode = reqMode;
            if (commState == CommState::Idle) {
                ManualRequest_PrepareAndSend(); // this will start send the request
            }
            else {
                manualRequest_Pending = true;
            }
        }
        bool REGO600Driver::ManualRequest_PrepareAndSend() {
            if (manualRequest_Mode == RequestMode::Lcd) {
                
                OpCodeInfo info = getCmdInfo(CommandID::ReadDisplay);
                uartTxBuffer[1] = info.opcode; //static_cast<uint8_t>(OpCodes::ReadDisplay);
                readLCD_RowIndex = 0;
                currentExpectedRxLength = 42;
                SetRequestAddr(0x00);
                uartTxBuffer[8] = 0x00;

                // initialize it, if this is the first use
                if (readLCD_Text == nullptr) { 
                    readLCD_Text = new char[20*4+1]();
                }

            } else if (manualRequest_Mode == RequestMode::FrontPanelLeds) {
                readFrontPanelLeds_Data = 0x00;
                OpCodeInfo info = getCmdInfo(CommandID::ReadFrontPanel);
                uartTxBuffer[1] = info.opcode; //static_cast<uint8_t>(OpCodes::ReadFrontPanel);
                currentExpectedRxLength = 5;
                readFrontPanelLedsIndex = 0;
                SetRequestAddr(0x12);
                CalcAndSetTxChecksum();
                
            } else if (manualRequest_Mode == RequestMode::OneTime && manualRequest != nullptr) {
                uartTxBuffer[1] = (uint8_t)manualRequest->info.opcode;
                currentExpectedRxLength = manualRequest->info.size;
                
                SetRequestAddr(manualRequest->def.address);

                if (manualRequest->response.value != nullptr) {
                    SetRequestData(manualRequest->response.value->toUInt());
                } else {
                    SetRequestData(0x0000); // not used here
                }
                CalcAndSetTxChecksum();
                
            } else {
                return false;
            }
            mode = manualRequest_Mode;
            ScheduleNextRequest();
            return true;
        }

        uint8_t* REGO600Driver::GetRawBytesBufferRef() {
            // initialize it, if this is the first use
            if (ramDumpBytes == nullptr) {
                ramDumpBytes = new uint8_t[20];
            }
            return ramDumpBytes;
        }

        bool REGO600Driver::OneTimeRequest(std::unique_ptr<Request> req, RequestCallback cb, void* cb_ctx) {
            if (cb == nullptr) {
                ErrorReport::DebugMessage(String(F("OneTimeRequest - callback cannot be nullptr")).c_str());
                return false; // no point if cb for some reason is nullptr
            }
            if (req->info.responseType != ResponseType::WriteConfirm) {
                return false; // no point if cb for some reason is nullptr
            }
            if (mode != RequestMode::RefreshLoop) { 
                ErrorReport::DebugMessage(String(F("OTReq - manual request allready in progress")).c_str());
                return false;
            }
            manualRequest_Callback = cb;
            manualRequest_Context = cb_ctx;
            manualRequest = std::move(req); // could also do req.release() to return the raw ptr, but then the raw ptr needs to be deleted when used

            ManualRequest_Schedule(RequestMode::OneTime);
            return true;
        }

        bool REGO600Driver::RequestWholeLCD(RequestCallback cb, void* cb_ctx) {
            if (cb == nullptr) {
                ErrorReport::DebugMessage(String(F("ReqLCD - callback cannot be nullptr")).c_str());
                return false; // no point if cb for some reason is nullptr
            }
            if (mode != RequestMode::RefreshLoop) { 
                ErrorReport::DebugMessage(String(F("ReqLCD - manual request allready in progress")).c_str());
                return false;
            }
            manualRequest_Callback = cb;
            manualRequest_Context = cb_ctx;
            ManualRequest_Schedule(RequestMode::Lcd);
            return true;
        }
        bool REGO600Driver::RequestFrontPanelLeds(RequestCallback cb, void* cb_ctx) {
            if (cb == nullptr) {
                ErrorReport::DebugMessage(String(F("ReqFrontPanelLeds - callback cannot be nullptr")).c_str());
                return false; // no point if cb for some reason is nullptr
            }
            if (mode != RequestMode::RefreshLoop) { 
                ErrorReport::DebugMessage(String(F("ReqFrontPanelLeds - manual request allready in progress")).c_str());
                return false;
            }
            manualRequest_Callback = cb;
            manualRequest_Context = cb_ctx;
            ManualRequest_Schedule(RequestMode::FrontPanelLeds);
            return true;
        }

        void REGO600Driver::RefreshLoop_SendCurrent() {
            Request* req = refreshLoopList[refreshLoopIndex];

            uartTxBuffer[1] = (uint8_t)req->info.opcode;
            SetRequestAddr(req->def.address);
            CalcAndSetTxChecksum();
            currentExpectedRxLength = req->info.size;
            ScheduleNextRequest();
        }

        void REGO600Driver::RefreshLoop_Restart() {
            lastUpdateMs = millis();
            refreshLoopIndex = 0;
            RefreshLoop_SendCurrent();   
        }

        void REGO600Driver::RefreshLoop_Continue() {
            if (refreshLoopIndex < (refreshLoopCount-1)) {
                refreshLoopIndex++;
                RefreshLoop_SendCurrent();
            } else {
                refreshLoopDoneFlag = true;
                // one loop done
                // exec some cb here, or set some flags
                commState = CommState::Idle;
            }
        }
        bool REGO600Driver::RefreshLoopDone() {
            if (refreshLoopDoneFlag == false) {
                return false;
            }
            refreshLoopDoneFlag = false;
            return true;
        }

        void REGO600Driver::RxDone_RefreshLoop() {
            if (refreshLoopList[refreshLoopIndex]->ValidateAndSetFromBuffer(uartRxBuffer) == Request::ValidateSetResult::Retry) {
                ErrorReport::DebugMessage(String(F("refreshLoopList RX - invalid value ")).c_str());
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
        void REGO600Driver::RxDone_LCD() {
            uint8_t tmp[20];
            if (!Request::unpack_nibbles(&uartRxBuffer[1], tmp, 20)) {
                ErrorReport::DebugMessage(String(F("LCD data corruption detected - discarding frame - ")).c_str());
                SendRequestFrameAndResetRx(); // retry @ LCD data error
                return;
            }
            
            memcpy(&readLCD_Text[readLCD_RowIndex*20], tmp, 20);
            
            if (readLCD_RowIndex == 3) { // this was the last row
                
                // execute a callback here
                if (manualRequest_Callback != nullptr) {
                    manualRequest_Callback(manualRequest_Context, readLCD_Text, manualRequest_Mode);
                } else {
                    ErrorReport::DebugMessage(String(F("LCD - mReqCB not set")).c_str());
                }
                ReturnToRefreshLoop();
            } else {
                readLCD_RowIndex++;
                uartTxBuffer[4] = readLCD_RowIndex;
                uartTxBuffer[8] = readLCD_RowIndex;
                ScheduleNextRequest();
            }
        }
        void REGO600Driver::RxDone_FrontPanelLeds() {
            if (uartRxBuffer[3] == 0x01) { 
                readFrontPanelLeds_Data |= 0x01;
            }
            if (readFrontPanelLedsIndex != 4) {
                readFrontPanelLedsIndex++;
                readFrontPanelLeds_Data <<= 1; // shift data to the right
                uartTxBuffer[4] = readFrontPanelLedsIndex + 0x12;
                uartTxBuffer[8] = readFrontPanelLedsIndex + 0x12;
                ScheduleNextRequest();
            } else {
                
                if (manualRequest_Callback != nullptr) {
                    manualRequest_Callback(manualRequest_Context, &readFrontPanelLeds_Data, manualRequest_Mode);
                } else {
                    ErrorReport::DebugMessage(String(F("FP - mReqCB not set")).c_str());
                }
                ReturnToRefreshLoop();
            }
        }
        void REGO600Driver::RxDone_OneTime() {
            if (manualRequest_Callback != nullptr) {
                // only set here, there is no point setting the data if there are no receiver
                // actually making the request in the first place when
                // the callback is not set is actually pointless

                Request::ValidateSetResult setRes = manualRequest->ValidateAndSetFromBuffer(uartRxBuffer);
                if (setRes == Request::ValidateSetResult::Retry) {
                    // try the request again
                    SendRequestFrameAndResetRx(); // retry @ OneTime value error
                    return;
                } else if (setRes == Request::ValidateSetResult::Success) {
                    manualRequest_Callback(manualRequest_Context, manualRequest.get(), manualRequest_Mode);
                }
            }
            else {
                ErrorReport::DebugMessage(String(F("OT - mReqCB not set")).c_str());
            }
            manualRequest.reset(); // free the current data
            ReturnToRefreshLoop();
        }

        void REGO600Driver::ReturnToRefreshLoop() {
            mode = RequestMode::RefreshLoop;
            manualRequest_Mode = RequestMode::RefreshLoop;
            RefreshLoop_Continue();
        }

        void REGO600Driver::SendRequestFrameAndResetRx() {
            lastRequestMs = millis();
            uartRxBufferIndex = 0;
            commState = CommState::AwaitingResponse;
            REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
        }

        void REGO600Driver::SetRequestAddr(uint16_t address) {
            uartTxBuffer[2] = (address >> 14) & 0x7F;
            uartTxBuffer[3] = (address >> 7) & 0x7F;
            uartTxBuffer[4] = address & 0x7F;
            uartTxBuffer[5] = 0x00; // allways zero unless set
            uartTxBuffer[6] = 0x00; // allways zero unless set
            uartTxBuffer[7] = 0x00; // allways zero unless set
        }

        void REGO600Driver::SetRequestData(uint16_t data) {
            uartTxBuffer[5] = (data >> 14) & 0x7F;
            uartTxBuffer[6] = (data >> 7) & 0x7F;
            uartTxBuffer[7] = data & 0x7F;
        }

        void REGO600Driver::CalcAndSetTxChecksum() {
            uint8_t chksum = 0;
            for (int i=2;i<8;i++) {
                chksum ^= uartTxBuffer[i];
            }
            uartTxBuffer[8] = chksum;
        }

        bool REGO600Driver::CalcAndCompareRxDataChecksum() {
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

        uint16_t REGO600Driver::GetValueFromUartRxBuff() {
            return (uartRxBuffer[1] << 14) + (uartRxBuffer[2] << 7) + uartRxBuffer[3];
        }

        void REGO600Driver::FlushCleanUARTRxBuffer(REGO600_UART_TYPE& uart, size_t maxDrains) {
            size_t count = 0;
            while (uart.available() && count++ < maxDrains) {
                uart.read();
            }
        }
        void REGO600Driver::ScheduleNextRequest() {
            pendingRequestLastTime = millis();
            commState = CommState::PendingSend;
        }

        void REGO600Driver::ParseCurrentRxPacket() {
            FlushCleanUARTRxBuffer(REGO600_UART_TO_USE); // allways flush remaining garbage if any

            if (uartRxBuffer[0] == 0x40) {
                ErrorReport::DebugMessage(String(F("REGO600 RX failure: device returned error 0x40 in ACK, restarting request")).c_str());
                SendRequestFrameAndResetRx(); // retry @ start byte error
                return;
            }
            if (uartRxBuffer[0] != 0x01) {
                ErrorReport::DebugMessage((String(F("RX done - corrupted frame - start byte mismatch:")) + String(uartRxBuffer[0], 16)).c_str());
                SendRequestFrameAndResetRx(); // retry @ start byte error
                return;
            }

            if (CalcAndCompareRxDataChecksum() == false) {
                // try the request again
                ErrorReport::DebugMessage(String(F("manualReq RX - Checksum error")).c_str());
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
        void REGO600Driver::AwaitingResponseTask() {
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
                    commState = CommState::Idle;
                    mode = RequestMode::RefreshLoop;
                    FlushCleanUARTRxBuffer(REGO600_UART_TO_USE);
                    ErrorReport::DebugMessage(String(F("uartRxBuffer full - this is likely to a hw problem")).c_str());
                }
                lastRequestMs = millis();
            }
            // Timeout check after possible any rx
            unsigned long now = millis();
            if ((now - lastRequestMs) >= requestTimeoutMs) {

                ErrorReport::DebugMessage((String(F("REGO600-request-timeout @ index=")) + String(uartRxBufferIndex) + String(F(", expected length=")) + String(currentExpectedRxLength)).c_str());
                //GlobalLogger.Error(F("REGO600-request-timeout")); // only log to logger to not fill serial/websocket with stuff
                //DALHAL::WebSocketAPI::Broadcast("REGO600-request-timeout"); // not needed anymore as logging to GlobalLogger automatically do it on websocket as well
                
                FlushCleanUARTRxBuffer(REGO600_UART_TO_USE);
                SendRequestFrameAndResetRx(); // retry @ timeout
            }
            if (failsafeReadCount >= REGO600_UART_RX_MAX_FAILSAFECOUNT) {
                ErrorReport::DebugMessage(String(F("read failsafe overflow")).c_str());
            }
        }

        void REGO600Driver::loop() {

            switch (commState) {
                case CommState::AwaitingResponse: {
                    AwaitingResponseTask();                
                    break;
                }
                case CommState::PendingSend: {
                    unsigned long now = millis();
                    if ((now - pendingRequestLastTime) > pendingRequestDelayMs) {
                        SendRequestFrameAndResetRx();
                    }
                    break;
                }
                case CommState::Idle: {
                    //  here we just take care of any glitches and receive garbage data if any
                    FlushCleanUARTRxBuffer(REGO600_UART_TO_USE);
                    // take care of any pending manual requests directly
                    if (manualRequest_Pending) {
                        manualRequest_Pending = false;
                        if (ManualRequest_PrepareAndSend() == true) {
                            return;
                        }
                    }
                    if (refreshLoopList == nullptr) {
                        // nothing to do
                        return;
                    }
                    unsigned long now = millis();
                    if (now - lastUpdateMs >= refreshTimeMs) {
                        RefreshLoop_Restart(); // this will also take care of updating lastUpdateMs
                    }
                    break;
                }
                default: break;
            }
        }
    }
}