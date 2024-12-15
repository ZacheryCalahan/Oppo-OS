#include "headers/stdio.h"
#include "../../drivers/headers/uart.h"

void putc(const char c) {
    uart_transmit(c);
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
                    unsigned int value = va_arg(args, unsigned int);
                    utoa(value, buffer, 16);
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
            putc(*format);
        }
        format++;
    }
}