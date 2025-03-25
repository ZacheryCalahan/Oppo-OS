#include "headers/scanner.h"
#include "headers/stdlib.h"
#include "headers/string.h"
#include "headers/intutil.h"

char next_char(scanner *data) {
    char a = *(char*)(data->scanner_address);
    data->scanner_address++;
    return a;
}

char* next_string(scanner *data) {
    char* a = (char*)(data->scanner_address);
    uint32_t len = strlen(a);
    data->scanner_address += len + 1; // Account for the added null character.
    char* str = kmalloc((size_t) len);
    strcpy(str, a);
    return str;
}

uint8_t next_byte(scanner *data) {
    uint8_t a = *(uint8_t*)(data->scanner_address);
    data->scanner_address++;
    return a;
}

int8_t next_sbyte(scanner *data) {
    int8_t a = *(int8_t*)(data->scanner_address);
    data->scanner_address++;
    return a;
}

uint16_t next_short(scanner *data) {
    uint16_t a = *(uint16_t*)(data->scanner_address);
    data->scanner_address += 2;
    if (data->is_big_endian) {
        a = uint16_be_to_le(a);
    }

    return a;
}

int16_t next_sshort(scanner *data) {
    int16_t a = *(int16_t*)(data->scanner_address);
    data->scanner_address += 2;
    if (data->is_big_endian) {
        a = uint16_be_to_le((uint16_t) a);
    }

    return (int16_t) a;
}

uint32_t next_int(scanner *data) {
    uint32_t a = *(uint32_t*)(data->scanner_address);
    data->scanner_address += 4;
    if (data->is_big_endian) {
        a = uint32_be_to_le(a);
    }
    
    return a;
}

int32_t next_sint(scanner *data) {
    int32_t a = *(int32_t*)(data->scanner_address);
    data->scanner_address += 4;
    if (data->is_big_endian) {
        a = uint32_be_to_le((uint32_t) a);
    }
    
    return (int32_t) a;
}

uint64_t next_long(scanner *data) {
    uint64_t a = *(uint64_t*)(data->scanner_address);
    data->scanner_address += 8;
    if (data->is_big_endian) {
        a = uint64_be_to_le(a);
    }
    
    return a;
}

int64_t next_slong(scanner *data) {
    int64_t a = *(int64_t*)(data->scanner_address);
    data->scanner_address += 8;
    if (data->is_big_endian) {
        a = uint64_be_to_le((uint64_t) a);
    }
    
    return (int64_t) a;
}

// Aligns the scanner to a set alignment by advancing the scanner
void align_scanner(scanner *data, uint32_t alignment) {
    while ((uintptr_t) data->scanner_address % alignment != 0) {
        data->scanner_address++;
    }

    return;
}

void skip_bytes(scanner *data, uint32_t bytes) {
    data->scanner_address += bytes;
}

bool move_pointer(scanner *data, uintptr_t address) {
    if (address - (uintptr_t) data->scanner_address > data->length)
        data->scanner_address = (void*) address;
        return 0;
    return 1;
}

void* get_current_pointer(scanner *data) {
    return data->scanner_address;
}