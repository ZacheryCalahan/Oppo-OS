#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

// Returns a copy of a string
char* strcpy(char *dst, const char *src);

char* strncpy(char *dst, const char *src, size_t n);

// Concatenates two strings
char* strcat(char *dst, const char *src);

// Returns the length of a string
uint32_t strlen(char *a);

// Checks if two strings are equal
uint32_t strcmp(char *a, char *b); 

#endif