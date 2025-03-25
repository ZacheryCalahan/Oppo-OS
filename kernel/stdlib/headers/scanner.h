#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    void* start_address;
    void* scanner_address;
    bool is_big_endian;
    size_t length;
} scanner;

void align_scanner(scanner *data, uint32_t alignment);
void skip_bytes(scanner *data, uint32_t bytes);
bool move_pointer(scanner *data, uintptr_t address);
void* get_current_pointer(scanner *data);

char next_char(scanner *data);
char* next_string(scanner *data);

uint8_t next_byte(scanner *data);
uint16_t next_short(scanner *data);
uint32_t next_int(scanner *data);
uint64_t next_long(scanner *data);

int8_t next_sbyte(scanner *data);
int16_t next_sshort(scanner *data);
int32_t next_sint(scanner *data);
int64_t next_slong(scanner *data);

#endif