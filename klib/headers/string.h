#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

/**
 * Returns a copy of a string
 */
char* strcpy(char *dst, const char *src);

/**
 * Returns a copy of a string, with a given size
 */
char* strncpy(char *dst, const char *src, size_t n);

/**
 * Concatenates two strings
 */
char* strcat(char *dst, const char *src);

/**
 * Returns the length of a string
 */
uint32_t strlen(char *a);

/**
 *  Checks if two strings are equal
 */
uint32_t strcmp(char *a, char *b);

/**
 * Finds and returns the first occurance of `c` in `s`. Returns NULL ptr if `c` is not found in `s`.
 */
char* strchr(const char *s, char c);

/**
 * Returns the length of the inital substing of s1 that
 * consists of chars not in s2.
 */
uint32_t strcspn(const char *s1, const char *s2);

/**
 * Tokenizes a string into tokens seperated by delimiters. On initial usage, `*s` should be the string to tokenize,
 * and `NULL` on subsequent calls. 
 * 
 * @param save_ptr A pointer to keep track of the tokenizer's position.
 * @param s String to tokenize
 * @param delim The delimiting string
 * 
 * @returns The next token in the string, or `NULL` when no tokens remain.
 */
char* strtok_r(char *s, char *delim, char **save_ptr);

#endif