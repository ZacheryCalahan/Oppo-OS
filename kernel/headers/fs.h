/*
    Contains the abstraction layer between the underlying file system, and the user facing files/directories. 
    Changes here WILL break older programs. Extend, not rewrite, if possible. See VFS in *nix OS's.
*/

#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

struct FILE {
    int in_use;         // Intended to prevent writes over in use file handler. Should be handled by fs.h.
    uint64_t size;      // Size of the file in bytes.
    uint8_t attributes; // Attributes of the file.
    char data[];        // All data of the file.
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
void flush_file(const char* path, struct FILE *file);
#endif