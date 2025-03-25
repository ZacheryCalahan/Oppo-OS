#include "headers/intutil.h"

// Converts a big endian short to little endian.
uint16_t uint16_be_to_le(uint16_t a) {
    return  ((a & 0x00FF) << 8) |
            ((a & 0xFF00) >> 8);
}

// Converts a big endian int number to little endian.
uint32_t uint32_be_to_le(uint32_t a) {
    return  ((a >> 24) & 0xff) |
            ((a << 8) & 0xff0000) |
            ((a >> 8) & 0xff00) |
            ((a << 24) & 0xff000000);
}

// Converts a big endian long to little endian.
uint64_t uint64_be_to_le(uint64_t a) {
    return  ((a & 0x00000000000000FF) << 56) | 
            ((a & 0x000000000000FF00) << 40) | 
            ((a & 0x0000000000FF0000) << 24) | 
            ((a & 0x00000000FF000000) << 8)  |
            ((a & 0x000000FF00000000) >> 8)  |
            ((a & 0x0000FF0000000000) >> 24) | 
            ((a & 0x00FF000000000000) >> 40) | 
            ((a & 0xFF00000000000000) >> 56);
}