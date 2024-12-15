#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stdarg.h>

void putc(const char c);
void puts(const char *str);

void utoa(uint64_t value, char *buffer, int base);
void vprintf(const char *format, va_list args);
void printf(char *format, ...);

#endif