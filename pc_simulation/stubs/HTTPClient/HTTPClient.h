#pragma once
#include <string>
#include <WiFiClient.h>

class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();

    bool begin(const std::string& url);
    bool begin(WiFiClient& client, const char* url); // ESP32/ESP8266 overload
    void end();
    int GET();
    int POST(const std::string& payload);
    std::string getString();
    void addHeader(const std::string& name, const std::string& value);
};
