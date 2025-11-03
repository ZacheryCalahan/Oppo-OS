#include "string.h"

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

char* strchr(const char *s, char c) {
    char x = c;
    if (s == NULL) 
        return NULL;
    
    for (;;) {
        if (*s == x)
            return (char *) s;
        else if (*s == '\0')
            return NULL;
        else
            s++;
    }
}

uint32_t strcspn(const char *s1, const char *s2) {
    uint32_t len;
    for (len = 0; s1[len] != '\0'; len++) 
        if (strchr(s2, s1[len]) != NULL)
            break;
    return len;
}

char* strtok_r(char *s, const char *delim, char **save_ptr) {
    char *token;

    if (delim == NULL)
        return NULL;
    if (s == NULL)
        s = *save_ptr;
    if (s == NULL)
        return NULL;

    // Skip delims at our current pos
    while (strchr(delim, *s) != NULL) {
        if (*s == '\0') {
            *save_ptr = s;
            return NULL;
        }

        s++;
    }

    // Skip any non-delims up until end of string
    token = s;
    while (strchr(delim, *s) == NULL)
        s++;

    if (*s != '\0') {
        *s = '\0';
        *save_ptr = s + 1;
    } else {
        *save_ptr = s;
    }
        
    return token;
    

}