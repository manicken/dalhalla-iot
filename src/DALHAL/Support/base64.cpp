
#include "base64.h"

#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

int b64urlCharToVal(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '-') return 62;
    if (c == '_') return 63;
    return -1; // invalid
}

const char* encodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int b64urlDecode(uint8_t *dst, size_t dstSize, const char *src) {
    uint8_t *o = dst;
    uint8_t *end = dst + dstSize;

    int val = 0;
    int valb = -8;

    for (const char *c = src; *c; c++) {
        if (*c == '\n' || *c == '\r' || *c == ' ')
            continue;

        if (*c == '=')  // padding → stop decoding
            break;

        int d = b64urlCharToVal((uint8_t)*c);
        if (d < 0)
            return -1; // invalid character → fail

        val = (val << 6) | d;
        valb += 6;

        if (valb >= 0) {
            if (o >= end)
                return -2; // output buffer too small

            *o++ = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }
    *o = '\0'; // null terminate
    return o - dst; // number of bytes written
}

int b64urlDecode(uint8_t *dst, size_t dstSize, const DALHAL::ZeroCopyString& src) {
    uint8_t *o = dst;
    uint8_t *end = dst + dstSize;

    int val = 0;
    int valb = -8;
    const char* srcEnd = src.end;
    for (const char *c = src.start; *c && c < srcEnd; c++) {
        if (*c == '\n' || *c == '\r' || *c == ' ')
            continue;

        if (*c == '=')  // padding → stop decoding
            break;

        int d = b64urlCharToVal((uint8_t)*c);
        if (d < 0)
            return -1; // invalid character → fail

        val = (val << 6) | d;
        valb += 6;

        if (valb >= 0) {
            if (o >= end)
                return -2; // output buffer too small

            *o++ = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }
    *o = '\0'; // null terminate
    return o - dst; // number of bytes written
}


String b64urlEncode(const uint8_t *data, size_t len) {
    
    String out;
    out.reserve(((len + 2) / 3) * 4);

    int val = 0, valb = -6;
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            out += encodeTable[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) out += encodeTable[((val << 8) >> (valb + 8)) & 0x3F];

    // No padding for URL-safe variant
    return out;
}

String b64urlEncode(const char *str) {
    return b64urlEncode((const uint8_t*)str, strlen(str));
}