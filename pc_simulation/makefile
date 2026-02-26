#for windows builds
#
#for linux builds


ifeq ($(OS),Windows_NT)
	CXX = g++
    CXXFLAGS = -std=c++17 -Wall -g -O0 -D_WIN32 -D_WIN32_WINNT=0x0A00
    LDFLAGS = -lwinhttp
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
	./stubs/Arduino \
	./ports/IPAddress \
	./stubs/PrintStream \
	./cpp-httplib \
	../.pio/libdeps/esp32dev/ArduinoJson/src \
	../lib/PubSubClient/src \
	./ports/Ticker \
	./ports/LittleFS \
	./ports/WiFiClient \
	./ports/HTTPClient \
	./ports/WiFi \
	./stubs/DHTesp \
	./stubs/OneWire \
	./stubs/DallasTemperature \
	./stubs/WS2812FX \
	./stubs/Adafruit_GFX \
	./stubs/Adafruit_SSD1306 \
	./stubs/Wire
else
INCLUDE_DIRS = \
	./stubs/Arduino \
	./ports/IPAddress \
	./stubs/PrintStream \
	../.pio/libdeps/esp32dev/ArduinoJson/src \
	../lib/PubSubClient/src \
	./ports/Ticker \
	./ports/LittleFS \
	./ports/WiFiClient \
	./ports/HTTPClient \
	./ports/WiFi \
	./stubs/DHTesp \
	./stubs/OneWire \
	./stubs/DallasTemperature \
	./stubs/WS2812FX \
	./stubs/Adafruit_GFX \
	./stubs/Adafruit_SSD1306 \
	./stubs/Wire
endif
INCLUDES = $(foreach dir,$(INCLUDE_DIRS),-I$(dir))
#	
# Source files
SRC = \
	./stubs/Arduino/Arduino.cpp \
    ./ports/IPAddress/IPAddress.cpp \
	$(wildcard ./stubs/PrintStream/*.cpp) \
    ../lib/PubSubClient/src/PubSubClient.cpp \
	./ports/DALHAL_REST/DALHAL_REST.cpp \
	$(wildcard *.cpp) \
	./ports/Ticker/Ticker.cpp \
	./ports/LittleFS/LittleFS.cpp \
	./ports/LittleFS/LittleFS_ext.cpp \
	./ports/WiFiClient/WiFiClient.cpp \
	./ports/HTTPClient/HTTPClient.cpp \
	./ports/WiFi/WiFi.cpp \
	./stubs/DHTesp/DHTesp.cpp \
	./stubs/OneWire/OneWire.cpp \
	./stubs/DallasTemperature/DallasTemperature.cpp \
	./stubs/Wire/Wire.cpp \
	./stubs/WS2812FX/WS2812FX.cpp \
	./stubs/Adafruit_SSD1306/Adafruit_SSD1306.cpp \
	../src/Drivers/REGO600.cpp \
	../src/Drivers/RF433.cpp \
	$(wildcard ../src/DALHAL/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/CoreDevices/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/Display_SSD1306/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/OneWireTemp/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/REGO600/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/RF433/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/Script/*.cpp) \
	$(wildcard ../src/DALHAL/Devices/HomeAssistant/*.cpp) \
	$(wildcard ../src/DALHAL/ScriptEngine/*.cpp) \
	$(wildcard ../src/DALHAL/ScriptEngine/Parser/*.cpp) \
	$(wildcard ../src/DALHAL/ScriptEngine/Runtime/*.cpp) \
	../src/Support/CharArrayHelpers.cpp \
	../src/Support/Logger.cpp \
	../src/Support/ConvertHelper.cpp

# Files to exclude (webserver-related)
EXCLUDE_SRC = \
	../src/DALHAL/DALHAL_REST.cpp \
	../src/DALHAL/DALHAL.cpp
	
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
