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

char* strncpy(char *dst, const char *src, size_t n) {
    char *p = dst;
    for (size_t i = 0; i < n; i++) {
        *p = *src;
        if (*src == '\0') return dst;
        p++;
        src++;
    }

    return dst;
}

uint32_t strcmp(char *a, char *b) {
    for (int i = 0; ; i++) {
        if (a[i] == '\0' && b[i] == '\0')
            return 0;

        char diff = a[i] - b[i];
        if (diff)
            return diff;
    }
}