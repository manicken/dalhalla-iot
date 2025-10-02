#pragma once

#include <Arduino.h>

int b64urlDecode(uint8_t *dst, const char *src);
String b64urlEncode(const uint8_t *data, size_t len);
String b64urlEncode(const char *str);