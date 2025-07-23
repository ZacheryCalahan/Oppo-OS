/*
    Contains the abstraction layer between the underlying file system, and the user facing files/directories. 
    Changes here WILL break older programs. Extend, not rewrite, if possible.
*/

#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

struct FILE {
    int in_use;
    uint64_t size;
    char data[];
};

struct FILE *open_file(const char* path);
void close_file(struct FILE *file);

#endif