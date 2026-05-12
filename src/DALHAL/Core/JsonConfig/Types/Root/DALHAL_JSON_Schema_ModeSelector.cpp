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

#include "DALHAL_JSON_Schema_ModeSelector.h"

#include <stdlib.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        int ModeSelector::evaluate(const ModeSelector* modes, const JsonVariant& j) {
            int matchedMode = -1;
            for (int i = 0; modes[i].name != nullptr; ++i) {
                const ModeSelector& mode = modes[i];
                bool modeValid = true;
                for (int c = 0; mode.conjunctions[c].fieldRef != nullptr; ++c) {
                    const ModeConjunctionDefine& conj = mode.conjunctions[c];
                    bool exists = false;
                    if (conj.fieldRef->type == FieldType::OneOfFieldsGroup) {
                        const SchemaOneOfFieldsGroup* group = static_cast<const SchemaOneOfFieldsGroup*>(conj.fieldRef);

                        for (int g = 0; group->fields[g] != nullptr; ++g) {
                            if (j.containsKey(group->fields[g]->name)) {
                                exists = true;
                                break;
                            }
                        }
                    }
                    else if (conj.fieldRef->type == FieldType::AllOfFieldsGroup) {
                        const SchemaAllOfFieldsGroup* group = static_cast<const SchemaAllOfFieldsGroup*>(conj.fieldRef);
                        int found = 0;
                        int total = 0;

                        for (int g = 0; group->fields[g] != nullptr; ++g) {
                            total++;
                            if (j.containsKey(group->fields[g]->name)) {
                                found++;
                            }
                        }
                        // ✅ group "exists" ONLY if fully present
                        exists = (found == total && total > 0);
                    } else {
                        exists = j.containsKey(conj.fieldRef->name);
                    }
                    if (conj.required != exists) {
                        modeValid = false;
                        break;
                    }
                }
                if (modeValid) {
                    if (matchedMode != -1) {
                        // multiple modes match -> ambiguous
                        return -2;
                    }
                    matchedMode = i;
                }
            }

            return matchedMode;
        }

        bool ModeSelector::Apply(const ModeSelector* modes, const DALHAL::DeviceCreateContext& createCtx, void* outStruct) {
            int modeIndex = evaluate(modes, *createCtx.jsonObjItem);
            if (modeIndex < 0) {
                return false;
            }
            modes[modeIndex].apply(createCtx, outStruct);
            return true;
        }

        void ModeSelector::ToJson(const ModeSelector* modes, std::string& out)
        {
            ToJsonString::appendKey(out, F("modes"));
            out += '[';
            bool firstMode = true;
            for (size_t i = 0; modes[i].name; ++i) {
                const auto& mode = modes[i];
                if (!firstMode) { out += ','; }
                else { firstMode = false; }
                out += '{';
                // mode name
                ToJsonString::appendString(out, F("name"), mode.name ? mode.name : "null");
                // conjunctions
                out += ',';
                ToJsonString::appendKey(out, F("conjunctions"));
                const auto* conj = mode.conjunctions;
                if (conj == nullptr) {
                    out += "null"; // a empty array mean something different
                    out += '}';
                    continue;
                }
                out += '[';
                bool firstConj = true;
                for (size_t j = 0; conj[j].fieldRef; ++j) {
                    const auto& c = conj[j];
                    // skip invalid entries safely
                    if (!c.fieldRef || !c.fieldRef->name) continue;
                    if (!firstConj) { out += ','; }
                    else { firstConj = false; }
                    out += '{';
                    ToJsonString::appendString(out, F("name"), c.fieldRef->name);
                    out += ',';
                    ToJsonString::appendBool(out, F("required"), c.required);
                    out += '}';
                }
                out += ']';
                out += '}';
            }
            out += ']';
        }

    }

}