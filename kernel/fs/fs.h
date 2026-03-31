/*
    Contains the abstraction layer between the underlying file system, and the user facing files/directories. 
    Changes here WILL break older programs. Extend, not rewrite, if possible. See VFS in *nix OS's.
*/

#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

#include "../../drivers/fs/ztfs.h"

struct FILE {
    uint64_t file_pointer;      // The current offset into the file being read
    struct ztfs_entry *entry;   // File metadata
    uint32_t reference_count;   // The amount of references to this file
};


/**
 * Reads up to the byte count in size into buf from the file pointed to by the file_desc.
 * Returns a negative value if an error occurs, otherwise returns the number of bytes read.
 * 
 * @param file_desc The file descriptor to read from
 * @param buf Storage for what's read from the file
 * @param size Amount of bytes to read up to
 * 
 * @returns Amount of bytes read, 0 if EOF.
 */
int64_t kfread(int32_t file_desc, void *buf, uint32_t size);

/**
 * Retrieves a file from the filesystem for user use.
 * 
 * @param path The path to the file.
 * @return File descriptor number representing the file in memory.
 */
int32_t kfopen(const char* path);

/**
 * Closes a file and returns the file space back to the allocator.
 * 
 * @param file Pointer to the `struct file`.
 */
void kfclose(int32_t file_desc);

/**
 * Writeback to the file pointed to by `struct FILE file`.
 * 
 * @param file Pointer to the FILE.
 */
void kfflush(int32_t file_desc);

#endif