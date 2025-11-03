/*
    Contains the abstraction layer between the underlying file system, and the user facing files/directories. 
    Changes here WILL break older programs. Extend, not rewrite, if possible. See VFS in *nix OS's.
*/

#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

#include "../../drivers/fs/ext2.h"

typedef int32_t filedec_t;     // File descriptor (index into file desc. table)

struct FILE {
    uint64_t file_pointer;      // The current offset into the file being read
    uint16_t access_mode;       // The permissions of the file
    struct inode *inode;        // The inode of the file
    uint32_t refrence_count;    // The amount of references to this file
};

enum FILE_ACCESS_PERMISSIONS {
    O_RO        = 1, // Read only
    O_WO        = 2, // Write only
    O_RW        = 3, // Read/Write
    O_CREATE    = 4, // Create if not existing
    O_MKFILE_W  = 6, // Shortcut to create and write file
    O_MKFILE_WR = 7, // Shortcut to create and W/R file.
};

enum FS_FILE_ERR {
    FS_FILE_NOT_FOUND = -1,
    FS_NO_OPEN_DESCRIPTORS = -2,
    FS_INVALID_PERMS = -3,
    
    
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
int64_t read_file(filedec_t file_desc, void *buf, uint32_t size);

/**
 * Retrieves a file from the filesystem for user use.
 * 
 * @param path The path to the file.
 * @return File descriptor number representing the file in memory.
 */
filedec_t open_file(const char* path, enum FILE_ACCESS_PERMISSIONS perms);

/**
 * Closes a file and returns the file space back to the allocator.
 * 
 * @param file Pointer to the `struct file`.
 */
void close_file(filedec_t file_desc);

/**
 * Writeback to the file pointed to by `struct FILE file`.
 * 
 * @param file Pointer to the FILE.
 */
void flush_file(filedec_t file_desc);

#endif