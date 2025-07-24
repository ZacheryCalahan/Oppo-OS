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


/**
 * Retrieves a file from the filesystem for user use.
 * 
 * @param path The path to the file.
 * @return `struct FILE` containing file data.
 */
struct FILE *open_file(const char* path);

/**
 * Closes a file and returns the file space back to the allocator.
 * 
 * @param file Pointer to the `struct file`.
 */
void close_file(struct FILE *file);

/**
 * Writeback to the file pointed to by `struct FILE file`.
 * 
 * @param path Path to the file.
 * @param file Pointer to the FILE.
 */
void write_file(const char* path, struct FILE *file);
#endif