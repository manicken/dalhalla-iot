# Dalhalla-IO – Modular IoT Framework

## Overview

I'm developing an **IoT framework** primarily targeting **ESP8266** and **ESP32** microcontrollers.  
However, the framework itself is **not bound to any specific hardware** — most of the development was actually done on **Windows (MinGW)** and later **Linux**, to take advantage of advanced debugging tools that helped isolate a **Heisenbug**.

Future candidates for porting include **Raspberry Pi Pico** and **Teensy**.

Currently, the project uses the **Arduino framework** as a base, but there’s essentially nothing preventing it from running **fully standalone**, apart from needing to provide custom drivers.

The **WiFiManager library** is used to easily flash multiple devices with firmware and register them on a network.  
This part may later evolve into something more like **Tasmota** or **ESPEasy**, simplifying device initialization.

---

## Key Features

- **Modular and lightweight architecture** — significantly smaller codebase than ESPEasy or Tasmota.
  - ~13,000 lines of code (VS Code Counter)
  - Tasmota: ~240,000 LOC
  - ESPEasy: ~200,000 LOC
- On ESP32, current firmware uses ~100 kB of flash (with room for optimization).
- **HAL/DAL configuration via JSON**, mapped to an internal tree structure.
- Devices can be added/removed **without firmware rebuild** or even restarting the system.
- Unified **device abstraction layer** — each device has a unique **UID** (8-character ASCII, mapped internally to `uint64_t` for fast access).
- All devices derive from a common `Device` base class with virtual functions.
- Optional **Device Capabilities interface** planned to auto-generate documentation for each device in the GUI.

---

## JSON Configuration

Configuration is hierarchical (tree-based), supporting both **physical** and **logical/virtual** devices.

- Address paths use `:` (colon) as a separator — safe for HTTP URLs.
- Example:  
  `root:1wire:sensor1:temp`

### Example: 1-Wire Bus Groups

- Multiple 1-Wire buses can be grouped.
- Supports synchronized measurements across sensors:
  - Send `start measure` to all devices.
  - Wait defined interval.
  - Read all values simultaneously.
- Supports `refreshTime` per group or per individual device.

---

## Core Data Types

### `HALValue`
A generic type handling integers, unsigned integers, and floats consistently, simplifying read/write/compare logic.

### `ZeroCopyString`
A custom lightweight string type (similar to `std::string_view`) with additional utility:
- `SplitOffHead(char delimiter)` – substring extraction without copying.
- Direct numeric conversion (`int32`, `uint32`, `float`).
- Hex values (`0x` prefix) auto-detected.
- Used extensively to minimize heap fragmentation.

---

## Script Engine

Inspired by **ESPEasy Rules**, with plans for **event-driven execution** (not yet implemented).

Scripts interact with devices via their UIDs, as defined in the JSON configuration.

**Syntax Highlights:**
- Conditionals (`if`, `then`, `elseif`, `else`, `endif`)
- Event blocks (`on`, `endon`) — planned.
- Newlines normalized to `\n`
- Supports `//` and `/* ... */` comments.
- Identifiers may include letters, digits, `_ : #`
- Numbers recognized automatically (`int`, `float`, hex).

**Example reserved keywords:**  
`if, then, do, elseif, else, endif, on, endon`

**Virtual devices** act as internal variables for flexible data handling.

---

## File Handling & Configuration Reload

- **Web-based file management** via FSBrowser for editing config and scripts.
- `CommandExecutor` (`HAL_JSON_CommandExecutor.cpp`) handles:
  - Config/script reload
  - Device commands
  - External access via REST

### REST API (Port 82)

| Endpoint | Description |
|-----------|-------------|
| `/reloadcfg` | Reload HAL configuration |
| `/scripts/reload` | Reload scripts |
| `/scripts/start` | Start script execution |
| `/scripts/stop` | Stop scripts |
| `/getAvailableGPIOs` | List GPIOs |
| `/printDevices` | Print all devices |
| `/printLog` | Show system log |
| `/exec/<device_uid>` | Execute command on device |
| `/read/<type>/<device_uid>/<cmd>` | Read from device |
| `/write/<type>/<device_uid>/<value>` | Write to device |

**Types:** `bool`, `uint`, `float`, `string`, `json`  
**Device-specific commands:** e.g. read `temp` from DHT sensor.

### Windows Environment
Uses a **command loop thread** to dispatch commands to the `CommandExecutor`.

---

## Architecture Details

### Timers

Uses a modified version of **TimeAlarms**, supporting:
- Individual parameter sets per alarm.
- Safe parameter casting using `OnTickExtParameters` base class.

> https://github.com/manicken/TimeAlarms

---

## Memory Optimization and Heap Management

### JSON Loading Process
1. File read into `const char*` buffer (for zero-copy parsing via ArduinoJSON).
2. JSON document created with 1.5× file size buffer.
3. Validation ensures all mandatory fields exist.
4. Valid entries counted, marked, and loaded into an internal device array.
5. Deallocation of buffer and JSON document.

Although pointer arrays help reduce fragmentation, complete deallocation leaves large gaps (“holes”).  
A potential future solution is to convert configuration into **BSON** to allow direct loading without reallocation.

### Script Loading
- All active scripts validated sequentially.
- Only valid scripts loaded into memory for execution.
- Comments stripped before memory allocation.
- Validated scripts compiled into optimized, pre-tokenized structures for fast runtime execution.

---

## Example Files

- **HAL config example:**  
  [hal_cfg.json](https://github.com/manicken/dalhalla-io/blob/main/hal_cfg.json)
- **TimerAlarms config example:**  
  [template.json](https://github.com/manicken/dalhalla-io/blob/main/template.json)
- **Script example:**  
  [script1.txt](https://github.com/manicken/dalhalla-io/blob/main/script1.txt)

---

## Webbased WiFi configurator

[Available here](https://github.com/manicken/dalhalla-iot/tree/main/InitialCfgGui)

- Introduces a HTML + JavaScript tool as an alternative to the AP-based web portal.
- Works in Chromium-based browsers (Edge, Opera, Chrome, etc.).
- Enables configuration of Wi-Fi settings and credentials via USB-serial connection, without needing to connect to the device's AP.
- **Log output:** displays messages printed by the device in real-time.
- **Command input:** allows sending commands directly to the device.



This provides a more flexible option for local setup and testing.

## Licensing

The project currently uses **GPLv3**, to ensure it remains **fully open-source**.  
However, device registries for `root` and `i2c` are under **MIT license**, allowing third-party device extensions without forced disclosure.

Open for discussion whether the entire project should remain GPLv3 or switch fully to MIT.

---

## Future Plans

- Full event-driven script execution.
- MQTT and WebSocket command sources.
- Integration with **Home Assistant**.
- More advanced web UI for configuration and file management.
- Optional BSON configuration storage to eliminate heap fragmentation.
- Extended device capability introspection and GUI auto-documentation.

---

## Purpose

> The main goal is to provide a **flexible architecture for IoT devices**,  
> where configuration and logic can be changed **without recompilation**.

**dependencies**<br>
special version of TimeAlarms<br>
that can have any amount of alarms (up to 255)<br>
and also can use parameters when calling handler functions<br>
note. this dependency is included in platformio.ini and is automatically installed<br>
https://github.com/manicken/TimeAlarms <br>