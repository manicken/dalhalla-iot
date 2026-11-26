#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

int b64urlDecode(uint8_t *dst, size_t dstSize, const char *src);
int b64urlDecode(uint8_t *dst, size_t dstSize, const DALHAL::ZeroCopyString& src);
std::string b64urlEncode(const uint8_t *data, size_t len);
std::string b64urlEncode(const char *str);