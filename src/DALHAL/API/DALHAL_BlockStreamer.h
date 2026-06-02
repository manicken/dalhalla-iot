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

#pragma once

#include <DALHAL/API/DALHAL_CommandExecutor.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>


namespace DALHAL {
    class BlockStreamer {
    public:
        enum class DataType {
            Json,
            PlainText
        };
        /**
         * Automatically starts the stream on construction.
         *
         * When the object goes out of scope, the stream is automatically closed.
         *
         * A new block can be started using `restart()` if needed, without
         * destroying the BlockStreamer instance.
         * 
         * note. tag MUST point to a valid null-terminated string
         * that remains valid for the entire lifetime of the BlockStreamer session.
         */
        BlockStreamer(CommandCallback cb, const char* tag, DataType dataType);
        BlockStreamer(const BlockStreamer&) = delete;
        BlockStreamer& operator=(const BlockStreamer&) = delete;
        BlockStreamer(BlockStreamer&&) = delete;

        ~BlockStreamer();

        // tag MUST point to a valid null-terminated string
        // that remains valid for the entire lifetime of the BlockStreamer session.
        void restart(const char* tag, DataType dataType);

        inline StringBuilderStreamer& writer() {
            return sbs;
        }

    private:

        CommandCallback _cb;
        StringBuilderStreamer sbs;
        

        const char* currentTag = nullptr;
        DataType currentDataType = DataType::PlainText;

        void sendHeaderFooter(const char* type);
        void start(const char* tag, DataType dataType);
        void flush();
        void end();

        const char* GetCurrentType_cStr();
    };

}