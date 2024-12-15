#include "headers/string.h"

uint32_t strlen(char *a) {
    uint32_t len = 0;
    while (*a != '\0') {
        len++;
        *a++;
    }

    return len;
}

char* strcat(char *dst, const char *src) {
    return strcpy(dst + strlen(dst), src);
}

char* strcpy(char *dst, const char *src) {
    char *p = dst;
    while ((*p ++ = *src++) != '\0')
    ;
    return dst;
}