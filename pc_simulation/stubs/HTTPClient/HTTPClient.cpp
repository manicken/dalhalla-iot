/*#include "HTTPClient.h"

HTTPClient::HTTPClient() {}
HTTPClient::~HTTPClient() {}

bool HTTPClient::begin(const std::string& url) { return true; }
bool HTTPClient::begin(WiFiClient& client, const char* url) { (void)client; (void)url; return true; }
void HTTPClient::end() {}
int HTTPClient::GET() { return 200; }
int HTTPClient::POST(const std::string& payload) { return 200; }
std::string HTTPClient::getString() { return ""; }
void HTTPClient::addHeader(const std::string& name, const std::string& value) {}
*/

#include "HTTPClient.h"
#ifdef _WIN32
#undef INPUT
#undef OUTPUT
#undef HIGH
#undef LOW
#include <windows.h>
#include <winhttp.h>
#endif
#include <iostream>

#pragma comment(lib, "winhttp.lib")

static std::wstring to_wide(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

HTTPClient::HTTPClient() : hSession(nullptr), hConnect(nullptr), hRequest(nullptr) {}

HTTPClient::~HTTPClient() { end(); }

bool HTTPClient::begin(const std::string& url) {
    end();

    std::wstring wurl = to_wide(url);
    // Split into host and path
    URL_COMPONENTS uc = { sizeof(uc) };
    wchar_t hostName[256];
    wchar_t urlPath[1024];
    uc.lpszHostName = hostName;
    uc.dwHostNameLength = _countof(hostName);
    uc.lpszUrlPath = urlPath;
    uc.dwUrlPathLength = _countof(urlPath);

    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &uc)) return false;

    hSession = WinHttpOpen(L"WinSimHTTPClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    hConnect = WinHttpConnect(hSession, hostName, uc.nPort, 0);
    if (!hConnect) return false;

    currentHost = hostName;
    currentPath = urlPath;
    return true;
}

bool HTTPClient::begin(WiFiClient&, const char* url) {
    return begin(std::string(url));
}

void HTTPClient::end() {
    if (hRequest) { WinHttpCloseHandle(hRequest); hRequest = nullptr; }
    if (hConnect) { WinHttpCloseHandle(hConnect); hConnect = nullptr; }
    if (hSession) { WinHttpCloseHandle(hSession); hSession = nullptr; }
}

int HTTPClient::GET() {
    if (!hConnect) return 0;
    hRequest = WinHttpOpenRequest(hConnect, L"GET", currentPath.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        (currentHost.find(L"https://") == 0) ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) return 0;

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        return 0;

    if (!WinHttpReceiveResponse(hRequest, NULL)) return 0;

    DWORD statusCode = 0, size = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);

    // Download the body
    responseBody.clear();
    DWORD dwSize = 0;
    do {
        DWORD dwDownloaded = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || dwSize == 0)
            break;

        std::string buffer(dwSize, 0);
        if (WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded))
            responseBody.append(buffer, 0, dwDownloaded);

    } while (dwSize > 0);

    return (int)statusCode;
}

int HTTPClient::POST(const std::string& payload) {
    if (!hConnect) return 0;

    hRequest = WinHttpOpenRequest(hConnect, L"POST", currentPath.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        (currentHost.find(L"https://") == 0) ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) return 0;

    WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/json\r\n", -1L, WINHTTP_ADDREQ_FLAG_ADD);

    std::wstring wpayload = to_wide(payload);

    if (!WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)wpayload.c_str(), (DWORD)(wpayload.size() * sizeof(wchar_t)),
        (DWORD)(wpayload.size() * sizeof(wchar_t)), 0))
        return 0;

    if (!WinHttpReceiveResponse(hRequest, NULL)) return 0;

    DWORD statusCode = 0, size = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);
    return (int)statusCode;
}

std::string HTTPClient::getString() {
    return responseBody;
}

void HTTPClient::addHeader(const std::string&, const std::string&) {
    // You can extend this to keep a header list and send it in GET/POST
}
