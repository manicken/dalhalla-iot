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

#include "DALHAL_CommonSchemas_Consumer.h"

//#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>
//#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>
//#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_ModeSelector.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_FieldConstraint.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/Types/DALHAL_Consumer.h>

#include "DALHAL_CommonSchemas_Time.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace CommonConsumer {

            constexpr SchemaStringUID_Path sourceField = { "source", FieldPolicy::Optional}; 
            constexpr SchemaStringUID_Path eventSourceField = { "event_source", FieldPolicy::Optional};

            constexpr const SchemaTypeBase* consumerFields[] = { &sourceField, &eventSourceField, &CommonTime::refreshTimeGroupFields, nullptr };
            constexpr SchemaFieldsGroup consumerFieldsGroup = {"consumer", consumerFields, Gui::UseInline};
            
            constexpr ModeConjunctionDefine timedRefreshModeConjunctions[] = {
                { &CommonTime::refreshTimeGroupFields, true },  // group must exist for this mode
                { &sourceField, true },            // source must exist
                { &eventSourceField, false },      // event_source must NOT exist
                { nullptr, false}
            };
            constexpr ModeConjunctionDefine eventModeConjunctions[] = {
                { &CommonTime::refreshTimeGroupFields, false },  // group must NOT exist for this mode
                { &sourceField, true },            // source must exist
                { &eventSourceField, true },      // event_source must exist
                { nullptr, false}
            };
            constexpr ModeConjunctionDefine manualModeConjunctions[] = {
                { &CommonTime::refreshTimeGroupFields, false },  // group must NOT exist for this mode
                { &sourceField, false },            // source must NOT exist
                { &eventSourceField, false },      // event_source must NOT exist
                { nullptr, false}
            };

            void Apply_TimedRefreshModeValues(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<ConsumerStruct*>(out);
                self->mode = DALHAL::Consumer::Mode::TimedRefresh;
                self->eventSource = nullptr;
                self->source = sourceField.ExtractFrom(*ctx.jsonObjItem);
                self->refreshtimems = CommonTime::refreshTimeGroupFields.ExtractFrom(*ctx.jsonObjItem).toUInt();
            }

            void Apply_EventModeValues(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<ConsumerStruct*>(out);
                self->mode = DALHAL::Consumer::Mode::Event;
                self->eventSource = eventSourceField.ExtractFrom(*ctx.jsonObjItem);
                self->source = sourceField.ExtractFrom(*ctx.jsonObjItem);
                self->refreshtimems = 0;
            }

            void Apply_ManualModeValues(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<ConsumerStruct*>(out);
                self->mode = DALHAL::Consumer::Mode::Manual;
                self->eventSource = nullptr;
                self->source = nullptr;
                self->refreshtimems = 0;
            }
            
            constexpr ModeSelector consumerDeviceModes[] = {
                {"timedRefresh", timedRefreshModeConjunctions, Apply_TimedRefreshModeValues},
                {"event", eventModeConjunctions, Apply_EventModeValues},
                {"manual", manualModeConjunctions, Apply_ManualModeValues},
                {nullptr, nullptr, nullptr}
            };

        }
        
    }

}