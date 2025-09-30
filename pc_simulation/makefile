#for windows builds
#
#for linux builds


ifeq ($(OS),Windows_NT)
	CXX = g++
    CXXFLAGS = -std=c++17 -Wall -g -O0 -D_WIN32 -D_WIN32_WINNT=0x0A00
    LDFLAGS =
else
    CXX = clang++
    CXXFLAGS = -std=c++17 -Wall -g -O0 -fsanitize=address -fno-omit-frame-pointer
    LDFLAGS = -fsanitize=address
endif


#LDFLAGS = -s

BUILD_DIR = build

#
# Header include path for ArduinoJson
ifeq ($(OS),Windows_NT)
INCLUDE_DIRS = \
	./cpp-httplib \
	../.pio/libdeps/esp32dev/ArduinoJson/src \
	./stubs/Arduino \
	./stubs/ESPAsyncWebServer \
	./stubs/Ticker \
	./stubs/LittleFS \
	./stubs/DHTesp \
	./stubs/OneWire \
	./stubs/DallasTemperature \
	./stubs/WS2812FX \
	./stubs/WiFiClient \
	./stubs/HTTPClient \
	./stubs/Adafruit_GFX \
	./stubs/Adafruit_SSD1306 \
	./stubs/Wire
else
INCLUDE_DIRS = \
	../.pio/libdeps/esp32dev/ArduinoJson/src \
	./stubs/Arduino \
	./stubs/ESPAsyncWebServer \
	./stubs/Ticker \
	./stubs/LittleFS \
	./stubs/DHTesp \
	./stubs/OneWire \
	./stubs/DallasTemperature \
	./stubs/WS2812FX \
	./stubs/WiFiClient \
	./stubs/HTTPClient \
	./stubs/Adafruit_GFX \
	./stubs/Adafruit_SSD1306 \
	./stubs/Wire
endif
INCLUDES = $(foreach dir,$(INCLUDE_DIRS),-I$(dir))
#	
# Source files
SRC = \
	./stubs/HAL_JSON_REST/HAL_JSON_REST.cpp \
	main.cpp \
	commandLoop.cpp \
	./stubs/Arduino/Arduino.cpp \
	./stubs/Ticker/Ticker.cpp \
	./stubs/LittleFS/LittleFS.cpp \
	./stubs/LittleFS/LittleFS_ext.cpp \
	./stubs/DHTesp/DHTesp.cpp \
	./stubs/OneWire/OneWire.cpp \
	./stubs/DallasTemperature/DallasTemperature.cpp \
	./stubs/Wire/Wire.cpp \
	./stubs/WS2812FX/WS2812FX.cpp \
	./stubs/WiFiClient/WiFiClient.cpp \
	./stubs/HTTPClient/HTTPClient.cpp \
	./stubs/Adafruit_SSD1306/Adafruit_SSD1306.cpp \
	../src/Drivers/REGO600.cpp \
	../src/Drivers/RF433.cpp \
	$(wildcard ../src/HAL_JSON/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/Display_SSD1306/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/OneWireTemp/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/REGO600/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/RF433/*.cpp) \
	$(wildcard ../src/HAL_JSON/ScriptEngine/*.cpp) \
	../src/Support/CharArrayHelpers.cpp \
	../src/Support/Logger.cpp \
	../src/Support/ConvertHelper.cpp

# Files to exclude (webserver-related)
EXCLUDE_SRC = \
	../src/HAL_JSON/HAL_JSON_REST.cpp \
	../src/HAL_JSON/HAL_JSON.cpp
	
# Remove excluded files from SRC
SRC := $(filter-out $(EXCLUDE_SRC), $(SRC))

ifeq ($(OS),Windows_NT)
    SOCKETS = -lws2_32
	EXECFILE = test.exe
else
    SOCKETS =
	EXECFILE = test
endif

# Convert each .cpp into a .o file in the build directory
OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

# Main executable
$(EXECFILE): $(OBJ)
	$(CXX) $(OBJ) $(INCLUDES) $(LDFLAGS) $(SOCKETS) -o $@

# Rule to compile each .cpp to .o in the build directory
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean target
.PHONY: clean
clean:
	rm -f $(OBJ) $(EXECFILE)
