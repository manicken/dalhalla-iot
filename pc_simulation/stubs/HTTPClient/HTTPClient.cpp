#include "HTTPClient.h"

HTTPClient::HTTPClient() {}
HTTPClient::~HTTPClient() {}

bool HTTPClient::begin(const std::string& url) { return true; }
bool HTTPClient::begin(WiFiClient& client, const char* url) { (void)client; (void)url; return true; }
void HTTPClient::end() {}
int HTTPClient::GET() { return 200; }
int HTTPClient::POST(const std::string& payload) { return 200; }
std::string HTTPClient::getString() { return ""; }
void HTTPClient::addHeader(const std::string& name, const std::string& value) {}
