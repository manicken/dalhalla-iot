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

#pragma once

#include <DALHAL/Config/DALHAL_BuildFlags.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveTypes.h>

#define DALHAL_REACTIVE_FEATURE_NONE           0x00000000

#define DALHAL_REACTIVE_FEATURE_BEGIN          0x00000001
#define DALHAL_REACTIVE_FEATURE_CYCLE_COMPLETE 0x00000002
#define DALHAL_REACTIVE_FEATURE_UNUSED_1       0x00000004
#define DALHAL_REACTIVE_FEATURE_UNUSED_2       0x00000008
#define DALHAL_REACTIVE_FEATURE_UNUSED_3       0x00000010
#define DALHAL_REACTIVE_FEATURE_UNUSED_4       0x00000020
#define DALHAL_REACTIVE_FEATURE_UNUSED_5       0x00000040
#define DALHAL_REACTIVE_FEATURE_UNUSED_6       0x00000080

#define DALHAL_REACTIVE_FEATURE_VALUE_CHANGE   0x00000100
#define DALHAL_REACTIVE_FEATURE_STATE_CHANGE   0x00000200
#define DALHAL_REACTIVE_FEATURE_UNUSED_7       0x00000400
#define DALHAL_REACTIVE_FEATURE_UNUSED_8       0x00000800
#define DALHAL_REACTIVE_FEATURE_UNUSED_9       0x00001000
#define DALHAL_REACTIVE_FEATURE_UNUSED_10      0x00002000
#define DALHAL_REACTIVE_FEATURE_UNUSED_11      0x00004000
#define DALHAL_REACTIVE_FEATURE_UNUSED_12      0x00008000

/* when the read function have executed successfully */
#define DALHAL_REACTIVE_FEATURE_READ           0x00010000
/* when the write function have executed successfully */
#define DALHAL_REACTIVE_FEATURE_WRITE          0x00020000
/* when the exec function have executed successfully */
#define DALHAL_REACTIVE_FEATURE_EXEC           0x00040000
#define DALHAL_REACTIVE_FEATURE_UNUSED_13      0x00080000
/* when the bracket read function have executed successfully */
#define DALHAL_REACTIVE_FEATURE_BRACKET_READ   0x00100000
/* when the bracket write function have executed successfully */
#define DALHAL_REACTIVE_FEATURE_BRACKET_WRITE  0x00200000
#define DALHAL_REACTIVE_FEATURE_UNUSED_14      0x00400000
#define DALHAL_REACTIVE_FEATURE_UNUSED_15      0x00800000

// errors
#define DALHAL_REACTIVE_FEATURE_READ_ERROR     0x01000000
#define DALHAL_REACTIVE_FEATURE_WRITE_ERROR    0x02000000
#define DALHAL_REACTIVE_FEATURE_EXEC_ERROR     0x04000000
#define DALHAL_REACTIVE_FEATURE_TIMEOUT        0x08000000

//enabler, this is used to keep the Reactive Feature enabled when using custom events
#define DALHAL_REACTIVE_FEATURE_CUSTOM         0x80000000

#define DALHAL_REACTIVE_FEATURE_RW ( DALHAL_REACTIVE_FEATURE_READ | DALHAL_REACTIVE_FEATURE_WRITE )

#define DALHAL_REACTIVE_FEATURE_ALL ( \
                                        DALHAL_REACTIVE_FEATURE_CUSTOM | \
                                        DALHAL_REACTIVE_FEATURE_BEGIN | \
                                        DALHAL_REACTIVE_FEATURE_CYCLE_COMPLETE | \
                                        DALHAL_REACTIVE_FEATURE_VALUE_CHANGE | \
                                        DALHAL_REACTIVE_FEATURE_STATE_CHANGE | \
                                        DALHAL_REACTIVE_FEATURE_READ | \
                                        DALHAL_REACTIVE_FEATURE_WRITE | \
                                        DALHAL_REACTIVE_FEATURE_EXEC | \
                                        DALHAL_REACTIVE_FEATURE_BRACKET_READ | \
                                        DALHAL_REACTIVE_FEATURE_BRACKET_WRITE | \
                                        DALHAL_REACTIVE_FEATURE_TIMEOUT | \
                                        DALHAL_REACTIVE_FEATURE_WRITE_ERROR | \
                                        DALHAL_REACTIVE_FEATURE_READ_ERROR | \
                                        DALHAL_REACTIVE_FEATURE_EXEC_ERROR \
                                    )

// easy copy/paste template
#define DALHAL_REACTIVE_CFG_ (DALHAL_REACTIVE_FEATURE_NONE)


// Actuators
#define DALHAL_REACTIVE_CFG_ACTUATOR              (DALHAL_REACTIVE_FEATURE_WRITE | DALHAL_REACTIVE_FEATURE_STATE_CHANGE | DALHAL_REACTIVE_FEATURE_CUSTOM) /* implemented */
#define DALHAL_REACTIVE_CFG_RELAY_LATCHING        (DALHAL_REACTIVE_FEATURE_WRITE | DALHAL_REACTIVE_FEATURE_STATE_CHANGE | DALHAL_REACTIVE_FEATURE_CUSTOM) /* implemented */
#define DALHAL_REACTIVE_CFG_LEDC_SERVO            (DALHAL_REACTIVE_FEATURE_WRITE) /* implemented */
// API

// DataStorageServices
#define DALHAL_REACTIVE_CFG_THINGSPEAK            (DALHAL_REACTIVE_FEATURE_EXEC) /* implemented, also as a consumer */
// DeviceContainer
#define DALHAL_REACTIVE_CFG_CONTAINER             (DALHAL_REACTIVE_FEATURE_NONE) /* nothing to implement */
// Display

// General Inputs
#define DALHAL_REACTIVE_CFG_DIGITAL_INPUT         (DALHAL_REACTIVE_FEATURE_READ)
#define DALHAL_REACTIVE_CFG_ANALOG_INPUT          (DALHAL_REACTIVE_FEATURE_VALUE_CHANGE | DALHAL_REACTIVE_FEATURE_READ)
#define DALHAL_REACTIVE_CFG_BUTTON                (DALHAL_REACTIVE_FEATURE_STATE_CHANGE | DALHAL_REACTIVE_FEATURE_CUSTOM)
// General Outputs
#define DALHAL_REACTIVE_CFG_DIGITAL_OUTPUT        (DALHAL_REACTIVE_FEATURE_WRITE)
#define DALHAL_REACTIVE_CFG_PWM_ANALOG_WRITE      (DALHAL_REACTIVE_FEATURE_WRITE)
#define DALHAL_REACTIVE_CFG_SINGLE_PULSE_OUTPUT   (DALHAL_REACTIVE_FEATURE_VALUE_CHANGE | DALHAL_REACTIVE_FEATURE_WRITE | DALHAL_REACTIVE_FEATURE_EXEC)
// HomeAssistant
#define DALHAL_REACTIVE_CFG_HOMEASSISTANT         (DALHAL_REACTIVE_FEATURE_NONE) /* nothing to implement */
// I2C
#define DALHAL_REACTIVE_CFG_I2C_BUS               (DALHAL_REACTIVE_FEATURE_READ | DALHAL_REACTIVE_FEATURE_WRITE)
// Lights
#define DALHAL_REACTIVE_CFG_WS2812                (DALHAL_REACTIVE_FEATURE_WRITE)
// REGO600
#define DALHAL_REACTIVE_CFG_REGO600               (DALHAL_REACTIVE_FEATURE_CYCLE_COMPLETE | DALHAL_REACTIVE_FEATURE_RW)
#define DALHAL_REACTIVE_CFG_REGO600_REGISTRY_ITEM (DALHAL_REACTIVE_FEATURE_VALUE_CHANGE)
// RF433
#define DALHAL_REACTIVE_CFG_TX433                 (DALHAL_REACTIVE_FEATURE_WRITE)
#define DALHAL_REACTIVE_CFG_TX433_UNIT            (DALHAL_REACTIVE_FEATURE_WRITE)
// Script
#define DALHAL_REACTIVE_CFG_SCRIPT_VARIABLE       (DALHAL_REACTIVE_FEATURE_VALUE_CHANGE) /* implemented */
#define DALHAL_REACTIVE_CFG_CONSTVAR              (DALHAL_REACTIVE_FEATURE_NONE) /* nothing to implement */
#define DALHAL_REACTIVE_CFG_WRITEVAR              (DALHAL_REACTIVE_FEATURE_VALUE_CHANGE)
#define DALHAL_REACTIVE_CFG_ARRAY                 (DALHAL_REACTIVE_FEATURE_BRACKET_READ | DALHAL_REACTIVE_FEATURE_BRACKET_WRITE)
// Sensors
#define DALHAL_REACTIVE_CFG_DHT                   (DALHAL_REACTIVE_FEATURE_VALUE_CHANGE)
#define DALHAL_REACTIVE_CFG_ONE_WIRE_TEMP_GROUP   (DALHAL_REACTIVE_FEATURE_CYCLE_COMPLETE)
#define DALHAL_REACTIVE_CFG_ONE_WIRE_TEMP_BUS     (DALHAL_REACTIVE_FEATURE_CYCLE_COMPLETE)
#define DALHAL_REACTIVE_CFG_ONE_WIRE_TEMP_DEVICE  (DALHAL_REACTIVE_FEATURE_VALUE_CHANGE)
// Templates
#define DALHAL_REACTIVE_CFG__TEMPLATE_              (DALHAL_REACTIVE_FEATURE_ALL)

#define HAS_REACTIVE(name, feature) (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_##feature)
#define USING_REACTIVE(name) (DALHAL_REACTIVE_CFG_##name)

/**
 * DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES
 *
 * When undefined, reactive device event tables use a type-safe template-based approach:
 *   - Each EventDescriptor stores a pointer-to-member (uint32_t CLASS_NAME::* counter)
 *     which provides full compile-time type safety and IDE support.
 *   - The compiler ensures that the event table matches the device class's member counters.
 *   - Recommended for development or small/medium projects where safety and clarity are important.
 *
 * When defined, a generic EventDescriptor table is used:
 *   - Counters are stored as offsets or void* pointers, allowing one table type for all devices.
 *   - Reduces template instantiations and binary bloat for large projects.
 *   - Some compile-time type checking is lost, but runtime safety is maintained via the reactive system macros.
 *
 * Choose based on whether you prioritize compile-time type safety or minimal template overhead.
 */


#ifndef DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES
#define DALHAL_DECLARE_REACTIVE_TABLE(CLASS_NAME, TABLE_NAME) static const EventDescriptorT<CLASS_NAME> TABLE_NAME[];
#define DALHAL_DEFINE_REACTIVE_TABLE(CLASS_NAME, TABLE_NAME)  const EventDescriptorT<CLASS_NAME> CLASS_NAME::TABLE_NAME[]
#else // future implementation for smaller but typeless builds
#define DALHAL_DECLARE_REACTIVE_TABLE(CLASS_NAME, TABLE_NAME) static const EventDescriptor TABLE_NAME[];
#define DALHAL_DEFINE_REACTIVE_TABLE(CLASS_NAME, TABLE_NAME)  const EventDescriptor CLASS_NAME::TABLE_NAME[]
#endif

#ifndef DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES
#define DALHAL_REACTIVE_ENTRY(CLASS_NAME, FEATURE_VAR_NAME) {#FEATURE_VAR_NAME, &CLASS_NAME::reactiveEventCounter##FEATURE_VAR_NAME}
#else
#define DALHAL_REACTIVE_ENTRY(CLASS_NAME, FEATURE_VAR_NAME) {#FEATURE_VAR_NAME, offsetof(CLASS_NAME, reactiveEventCounter##FEATURE_VAR_NAME)}
#endif

#define DALHAL_DECLARE_REACTIVE_FEATURE(FEATURE_NAME) \
private: \
    uint32_t reactiveEventCounter##FEATURE_NAME = 0; \
public: \
    inline void trigger##FEATURE_NAME() { reactiveEventCounter##FEATURE_NAME++; }

#define HAS_REACTIVE_CUSTOM(name)                (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_CUSTOM)
#define HAS_REACTIVE_BEGIN(name)                 (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_BEGIN)
#define HAS_REACTIVE_CYCLE_COMPLETE(name)        (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_CYCLE_COMPLETE)
#define HAS_REACTIVE_VALUE_CHANGE(name)          (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_VALUE_CHANGE)
#define HAS_REACTIVE_STATE_CHANGE(name)          (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_STATE_CHANGE)
#define HAS_REACTIVE_WRITE(name)                 (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_WRITE)
#define HAS_REACTIVE_READ(name)                  (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_READ)
#define HAS_REACTIVE_EXEC(name)                  (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_EXEC)
#define HAS_REACTIVE_BRACKET_READ(name)          (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_BRACKET_READ)
#define HAS_REACTIVE_BRACKET_WRITE(name)         (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_BRACKET_WRITE)
#define HAS_REACTIVE_TIMEOUT(name)               (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_TIMEOUT)
#define HAS_REACTIVE_READ_ERROR(name)            (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_READ_ERROR)
#define HAS_REACTIVE_WRITE_ERROR(name)           (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_WRITE_ERROR)
#define HAS_REACTIVE_EXEC_ERROR(name)            (DALHAL_REACTIVE_CFG_##name & DALHAL_REACTIVE_FEATURE_EXEC_ERROR)

#define REACTIVE_ENTRY_BEGIN(CLASS_NAME)          DALHAL_REACTIVE_ENTRY(CLASS_NAME, Begin)
#define REACTIVE_ENTRY_CYCLE_COMPLETE(CLASS_NAME) DALHAL_REACTIVE_ENTRY(CLASS_NAME, CycleComplete)
#define REACTIVE_ENTRY_VALUE_CHANGE(CLASS_NAME)   DALHAL_REACTIVE_ENTRY(CLASS_NAME, ValueChange)
#define REACTIVE_ENTRY_STATE_CHANGE(CLASS_NAME)   DALHAL_REACTIVE_ENTRY(CLASS_NAME, StateChange)
#define REACTIVE_ENTRY_WRITE(CLASS_NAME)          DALHAL_REACTIVE_ENTRY(CLASS_NAME, Write)
#define REACTIVE_ENTRY_READ(CLASS_NAME)           DALHAL_REACTIVE_ENTRY(CLASS_NAME, Read)
#define REACTIVE_ENTRY_EXEC(CLASS_NAME)           DALHAL_REACTIVE_ENTRY(CLASS_NAME, Exec)
#define REACTIVE_ENTRY_BRACKET_READ(CLASS_NAME)   DALHAL_REACTIVE_ENTRY(CLASS_NAME, BracketRead)
#define REACTIVE_ENTRY_BRACKET_WRITE(CLASS_NAME)  DALHAL_REACTIVE_ENTRY(CLASS_NAME, BracketWrite)
#define REACTIVE_ENTRY_TIMEOUT(CLASS_NAME)        DALHAL_REACTIVE_ENTRY(CLASS_NAME, Timeout)
#define REACTIVE_ENTRY_READ_ERROR(CLASS_NAME)     DALHAL_REACTIVE_ENTRY(CLASS_NAME, ReadError)
#define REACTIVE_ENTRY_WRITE_ERROR(CLASS_NAME)    DALHAL_REACTIVE_ENTRY(CLASS_NAME, WriteError)
#define REACTIVE_ENTRY_EXEC_ERROR(CLASS_NAME)     DALHAL_REACTIVE_ENTRY(CLASS_NAME, ExecError)

// Terminator for all tables
#ifndef DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES
#define REACTIVE_ENTRY__TERMINATOR_()             { nullptr, nullptr }
#else
#define REACTIVE_ENTRY__TERMINATOR_()             { nullptr, 0 }
#endif

#define REACTIVE_DECLARE_FEATURE_BEGIN()          DALHAL_DECLARE_REACTIVE_FEATURE(Begin)
#define REACTIVE_DECLARE_CYCLE_COMPLETE()         DALHAL_DECLARE_REACTIVE_FEATURE(CycleComplete)
#define REACTIVE_DECLARE_FEATURE_VALUE_CHANGE()   DALHAL_DECLARE_REACTIVE_FEATURE(ValueChange)
#define REACTIVE_DECLARE_FEATURE_STATE_CHANGE()   DALHAL_DECLARE_REACTIVE_FEATURE(StateChange)
#define REACTIVE_DECLARE_FEATURE_WRITE()          DALHAL_DECLARE_REACTIVE_FEATURE(Write)
#define REACTIVE_DECLARE_FEATURE_READ()           DALHAL_DECLARE_REACTIVE_FEATURE(Read)
#define REACTIVE_DECLARE_FEATURE_EXEC()           DALHAL_DECLARE_REACTIVE_FEATURE(Exec)
#define REACTIVE_DECLARE_FEATURE_BRACKET_READ()   DALHAL_DECLARE_REACTIVE_FEATURE(BracketRead)
#define REACTIVE_DECLARE_FEATURE_BRACKET_WRITE()  DALHAL_DECLARE_REACTIVE_FEATURE(BracketWrite)
#define REACTIVE_DECLARE_FEATURE_TIMEOUT()        DALHAL_DECLARE_REACTIVE_FEATURE(Timeout)
#define REACTIVE_DECLARE_FEATURE_READ_ERROR()     DALHAL_DECLARE_REACTIVE_FEATURE(ReadError)
#define REACTIVE_DECLARE_FEATURE_WRITE_ERROR()    DALHAL_DECLARE_REACTIVE_FEATURE(WriteError)
#define REACTIVE_DECLARE_FEATURE_EXEC_ERROR()     DALHAL_DECLARE_REACTIVE_FEATURE(ExecError)

template<typename T>
static void GenericValueCallback(void* ctx) {
    static_cast<T*>(ctx)->triggerValueChange();
}