#include "headers/stdio.h"
#include "../drivers/headers/uart.h"
#include "../drivers/headers/sbi.h"

void putc(const char c) {
    #ifdef SBI_H
    sbi_call(c, 0, 0, 0, 0, 0, 0, 1); // console putchar system call
    #else
    uart_transmit(c);
    #endif

}

void puts(const char *str) {
    while (*str != '\0') {
        putc(*str);
        str++;
    }
}

void utoa(uint64_t value, char *buffer, int base) {
    char *digits = "0123456789ABCDEF";
    char temp[32];
    int i = 0;

    // Special case for zero
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    // Convert the number to string
    while (value > 0) {
        temp[i++] = digits[value % base];
        value /= base;
    }

    // Reverse the result into the final buffer
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0'; // Terminate the string
}

void printf(char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void vprintf(const char *format, va_list args) {
    char buffer[32];

    while(*format) {
        if (*format == '%') {
            format++; // Skip this character
            switch (*format) {
                case 'd': { // Integer
                    int value = va_arg(args, int);
                    if (value < 0) {
                        putc('-');
                        value = -value;
                    }
                    utoa((uint64_t) value, buffer, 10);
                    puts(buffer);
                    break;
                }

                case 'x': { // Hexadecimal
                    uint64_t value = va_arg(args, uint64_t);
                    utoa(value, buffer, 16);
                    puts(buffer);
                    break;
                }

                case 'u': { // Unsigned Integer
                    int value = va_arg(args, int);
                    utoa((uint64_t) value, buffer, 10);
                    puts(buffer);
                    break;
                }

                case 's': { // String
                    const char *str = va_arg(args, const char *);
                    puts(str);
                    break;
                }

                case 'c': { // Character
                    char c = (char) va_arg(args, int);
                    putc(c);
                    break;
                }

                default: { // Unknown Format
                    putc('%');
                    putc(*format);
                    break;
                }
            } 
        } else {
            // Newline should also carraige return
            if (*format == '\n') {
                putc('\r');
            }
            putc(*format);
        }
        format++;
    }
}

void snprintf(char* buffer, size_t n, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, n, format, args);
    va_end(args);
}

void vsnprintf(char* buffer, size_t n, const char *format, va_list args) {
    char temp_buffer[32];
    uint32_t bufferIdx = 0;

    while(*format) {
        if (*format == '%') {
            format++; // Skip this character
            switch (*format) {
                case 'd': { // Integer
                    int value = va_arg(args, int);
                    if (value < 0) {
                        buffer[bufferIdx] = '-';
                        bufferIdx++;
                        value = -value;
                    }
                    utoa((uint64_t) value, temp_buffer, 10);
                    buffer[bufferIdx] = *temp_buffer;
                    bufferIdx += strlen(temp_buffer);
                    break;
                }

                case 'u': {
                    int value = va_arg(args, int);
                    utoa((uint64_t) value, temp_buffer, 10);
                    buffer[bufferIdx] = *temp_buffer;
                    bufferIdx += strlen(temp_buffer);
                    break;
                }

                case 'x': { // Hexadecimal
                    unsigned int value = va_arg(args, unsigned int);
                    utoa(value, temp_buffer, 16);
                    buffer[bufferIdx] = *temp_buffer;
                    bufferIdx += strlen(temp_buffer);
                    break;
                }

                case 's': { // String
                    const char *str = va_arg(args, const char *);
                    buffer[bufferIdx] = *str;
                    bufferIdx += strlen(str);
                    break;
                }

                case 'c': { // Character
                    char c = (char) va_arg(args, int);
                    buffer[bufferIdx] = c;
                    bufferIdx++;
                    break;
                }

                default: { // Unknown Format
                    putc('%');
                    putc(*format);

                    buffer[bufferIdx] = '%';
                    bufferIdx++;
                    buffer[bufferIdx] = *format;
                    bufferIdx++;
                    break;
                }
            } 
        } else {
            buffer[bufferIdx] = *format;
            bufferIdx++;
        }
        format++;
    }
}

