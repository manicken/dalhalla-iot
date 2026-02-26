
#include "base64.h"

int b64urlCharToVal(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '-') return 62;
    if (c == '_') return 63;
    return -1; // invalid
}

const char* encodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int b64urlDecode(uint8_t *dst, const char *src) {
    uint8_t *o = dst;
    int val = 0, valb = -8;

    for (const char *c = src; *c; c++) {
        if (*c == '\n' || *c == '\r' || *c == ' ') continue;

        int d = b64urlCharToVal((uint8_t)*c);
        if (d < 0) continue; // skip invalid chars

        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0) {
            *o++ = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }

    *o = '\0'; // null-terminate for C-strings
    return o - dst; // length of decoded data
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