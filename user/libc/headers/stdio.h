#ifndef U_STDIO_H
#define U_STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "string.h"

void puts(const char *str);

void utoa(uint64_t value, char *buffer, int base);
void vprintf(const char *format, va_list args);
void printf(char *format, ...);
void snprintf(char* buffer, size_t n, const char *format, ...);
void vsnprintf(char* buffer, size_t n, const char *format, va_list args);
#endif