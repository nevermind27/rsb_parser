#include "byteswap.h"

uint16_t byteswap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}

uint32_t byteswap32(uint32_t x) {
    return ((x >> 24) & 0x000000FF) |
        ((x >> 8) & 0x0000FF00) |
        ((x << 8) & 0x00FF0000) |
        ((x << 24) & 0xFF000000);
}

uint64_t byteswap64(uint64_t x) {
    return ((x >> 56) & 0x00000000000000FFULL) |
        ((x >> 40) & 0x000000000000FF00ULL) |
        ((x >> 24) & 0x0000000000FF0000ULL) |
        ((x >> 8) & 0x00000000FF000000ULL) |
        ((x << 8) & 0x000000FF00000000ULL) |
        ((x << 24) & 0x0000FF0000000000ULL) |
        ((x << 40) & 0x00FF000000000000ULL) |
        ((x << 56) & 0xFF00000000000000ULL);
}


uint16_t from_big_endian(uint16_t be_value) {
    if constexpr (std::endian::native == std::endian::big) {
        return be_value;
    }
    else {
        return byteswap16(be_value);
    }
}

uint32_t from_big_endian(uint32_t be_value) {
    if constexpr (std::endian::native == std::endian::big) {
        return be_value;
    }
    else {
        return byteswap32(be_value);
    }
}

uint64_t from_big_endian(uint64_t be_value) {
    if constexpr (std::endian::native == std::endian::big) {
        return be_value;
    }
    else {
        return byteswap64(be_value);
    }
}
