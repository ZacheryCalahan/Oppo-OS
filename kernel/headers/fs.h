/*
    Contains the abstraction layer between the underlying file system, and the user facing files/directories. 
    Changes here WILL break older programs. Extend, not rewrite, if possible. See VFS in *nix OS's.
*/

#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

#include "../../drivers/headers/ext2.h"

typedef uint32_t filedec_t;



struct FILE {
    uint64_t file_pointer;      // The current offset into the file being read
    uint16_t access_mode;       // The permissions of the file
    struct inode *inode;        // The inode of the file
    uint32_t refrence_count;    // The amount of references to this file
};
/**
 * 
 */
void* read_file(filedec_t file);

/**
 * Retrieves a file from the filesystem for user use.
 * 
 * @param path The path to the file.
 * @return File descriptor number representing the file in memory.
 */
int open_file(const char* path);

/**
 * Closes a file and returns the file space back to the allocator.
 * 
 * @param file Pointer to the `struct file`.
 */
void close_file(filedec_t file);

/**
 * Writeback to the file pointed to by `struct FILE file`.
 * 
 * @param file Pointer to the FILE.
 */
void flush_file(filedec_t file);

#endif