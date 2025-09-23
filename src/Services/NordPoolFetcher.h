#ifndef NORD_POOL_FETCHER_H
#define NORD_POOL_FETCHER_H

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
//#include <WiFiClientSecureBearSSL.h>
#include <WiFiClientSecure.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

namespace NPF {
    const char IRG_Root_X1 [] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

// Create a list of certificates with the server certificate
#if defined(ESP8266)
X509List cert(IRG_Root_X1);
#elif defined(ESP32)
char *cert = nullptr;
#endif
    std::string searchPatternInhtmlFromUrl() {
        std::string startPattern = "const data = {", endPattern = "};";
        std::string host = "www.elbruk.se";
        std::string url = "https://www.elbruk.se/timpriser-se4-malmo";
        std::string url2 = "https://www.elprisetjustnu.se/api/v1/prices/2024/02-29_SE3.json";
        //std::string url = "https://www.google.com/";
        const int chunkSize = 1024; // Adjust as needed
        char buffer[chunkSize];
        Serial1.println("before instance wificlient secure");
        WiFiClientSecure client;
#if defined(ESP8266)
        client.setBufferSizes(4096,256);
        Serial1.println("before setTrustacnchors");
        client.setTrustAnchors(&cert);
#elif defined(ESP32)

#endif
        //client.setInsecure();
        client.setTimeout(10000);
        //BearSSL::WiFiClientSecure client;// = new BearSSL::WiFiClientSecure();

        // Ignore SSL certificate validation
        //client.setInsecure();

        //WiFiClient client;
        
        HTTPClient http;
        Serial1.println("before http begin");
        //http.begin(client, host.c_str(), 433, url.c_str(), false);
        http.begin(client, url.c_str());
        http.setTimeout(10000);
        http.setUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36");
        int httpCode = http.GET();

        if (httpCode != HTTP_CODE_OK) {
            Serial1.print("Error on HTTP request: "); Serial1.println( httpCode);
            //Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            http.end();
            return "";
        }
        http.useHTTP10();
        http.setReuse(true);
        //WiFiClient* stream = http.getStreamPtr();
        
        
        std::string remainingData = ""; // To handle pattern matching across chunk boundaries
        std::string currentPattern = startPattern;
        std::string extractedData = "";
        bool foundStartPattern = false;
        while (client.connected()) {
            size_t bytesRead = client.readBytes(buffer, chunkSize);
            Serial1.println(bytesRead);
            if (bytesRead > 0) {
                std::string dataRead(buffer, bytesRead);
                Serial1.println(dataRead.c_str());
                /*std::string data = remainingData + dataRead;
                size_t pos = 0;
                pos = data.find(currentPattern);
                if (pos != std::string::npos) {
                    if (foundStartPattern == false) {
                        Serial1.println("found start pattern");
                        extractedData = data.substr(pos+currentPattern.size());
                        currentPattern = endPattern;
                        foundStartPattern = true;
                        remainingData = ""; // to ensure that duplicate pattern was not found if the pattern was at end of data
                    }
                    else {
                        extractedData += data.substr(0, pos);
                        break;
                    }
                }
                else {
                    if (foundStartPattern == true)
                        extractedData += dataRead;
                    remainingData = data.substr(std::max(static_cast<size_t>(0), data.size() - currentPattern.size())); // Preserve the last pattern-size characters for next iteration
                }*/
            }
            else
                break;
        }
        
        http.end();
        client.stop();
        return extractedData;
    }
/*
    void sourceFromChatGPT()
    {
        if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        // Read response data in chunks
        WiFiClient* stream = http.getStreamPtr();
        char buffer[128]; // Adjust buffer size as needed
        bool inTargetSection = false; // Flag to track if we're in the target section
        String extractedData; // To store the extracted data
        
        while (stream->available()) {
          size_t len = stream->readBytes(buffer, sizeof(buffer));
          for (size_t i = 0; i < len; i++) {
            if (!inTargetSection) {
              // Search for the start of the target section
              if (buffer[i] == '<' && buffer[i+1] == 'd' && buffer[i+2] == 'i' && buffer[i+3] == 'v' && buffer[i+4] == ' ' && buffer[i+5] == 'i' && buffer[i+6] == 'd' && buffer[i+7] == '=' && buffer[i+8] == '"' && buffer[i+9] == 'c' && buffer[i+10] == 'o' && buffer[i+11] == 'n' && buffer[i+12] == 't' && buffer[i+13] == 'e' && buffer[i+14] == 'n' && buffer[i+15] == 't' && buffer[i+16] == '"' && buffer[i+17] == '>') {
                inTargetSection = true;
                i += 17; // Move the index to the end of the opening tag
              }
            } else {
              // Search for the end of the target section
              if (buffer[i] == '<' && buffer[i+1] == '/' && buffer[i+2] == 'd' && buffer[i+3] == 'i' && buffer[i+4] == 'v' && buffer[i+5] == '>') {
                inTargetSection = false;
                // If found, stop reading further
                break;
              } else {
                extractedData += buffer[i]; // Store the character in the extracted data
              }
            }
          }
        }
        
        Serial.println(extractedData); // Print the extracted data
      }
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
    }*/
}

#endif