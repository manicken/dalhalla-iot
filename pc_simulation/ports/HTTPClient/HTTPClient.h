// HTTPClient.h
#pragma once
#include <string>
#include <WiFiClient.h>

#ifdef _WIN32
#undef INPUT
#undef OUTPUT
#undef HIGH
#undef LOW
#include <windows.h>
#include <winhttp.h>
#endif


class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();

    bool begin(const std::string& url);
    bool begin(WiFiClient& client, std::string url);
    void end();
    int GET();
    int POST(const std::string& payload);
    std::string getString();
    void addHeader(const std::string& name, const std::string& value);

private:
#ifdef _WIN32
    HINTERNET hSession;
    HINTERNET hConnect;
    HINTERNET hRequest;

    std::wstring currentHost;
    std::wstring currentPath;
    std::string responseBody;
#endif
};
