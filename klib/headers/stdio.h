#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "string.h"

void putc(const char c);
void puts(const char *str);
long getc(void);

void utoa(uint64_t value, char *buffer, int base); // Converts an unsigned integer to its char value
void vprintf(const char *format, va_list args);
void printf(char *format, ...);
void snprintf(char* buffer, size_t n, const char *format, ...);
void vsnprintf(char* buffer, size_t n, const char *format, va_list args);
#endif