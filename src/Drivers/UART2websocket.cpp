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

#include "UART2websocket.h"

UART2websocket::UART2websocket()
    : server(UART2WS_PORT), ws("/ws2uart"), UART(UART2WS_UART_TO_USE) // Init UART2 as HardwareSerial(2)
{}

void UART2websocket::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;

        if (info->opcode == WS_BINARY) {
            UART.write(data, len);
        }
    }
}

void UART2websocket::setup() {
#if defined(ESP32) && !defined(seeed_xiao_esp32c3)
    UART.begin(19200, SERIAL_8N1, 16, 17);  // Set correct RX/TX pins for UART2
#elif defined(ESP8266)
    UART.begin(19200, SERIAL_8N1);
#elif defined(seeed_xiao_esp32c3)
    UART.begin(19200);
#endif
    ws.onEvent([this](AsyncWebSocket *s, AsyncWebSocketClient *c, AwsEventType t, void *a, uint8_t *d, size_t l) {
        this->onWsEvent(s, c, t, a, d, l);
    });

    server.addHandler(&ws);
    server.begin();
}

void UART2websocket::task_loop() {
    // Read all available UART data
    while (UART.available()) {
        if (uartBufferIndex < UART2WS_BUFFER_SIZE) {
            uartBuffer[uartBufferIndex++] = UART.read();
            lastByteTime = millis(); // update last byte time
        }
    }

    // If buffer is not empty and timeout passed, send it
    if (uartBufferIndex > 0 && (millis() - lastByteTime > FLUSH_TIMEOUT_MS)) {
        ws.binaryAll(uartBuffer, uartBufferIndex);
        uartBufferIndex = 0;
    }
}
