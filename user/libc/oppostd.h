#ifndef OPPOSTD_H
#define OPPOSTD_H

/*
    All OS Specific stuff goes here, 
*/

#include <stdint.h>

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

struct inode {
    uint16_t permissions; // Contains the permissions and the type of inode
    uint16_t user_id;
    uint32_t low_size; // Lower 32 bits of the size of the inode
    uint32_t last_access_time;
    uint32_t creation_time;
    uint32_t last_modification_time;
    uint32_t deletion_time;
    uint16_t group_id;
    uint16_t count_hard_links; // Count of dir entries to the inode. When 0, the data blocks are marked as unallocated. 
    uint32_t count_disk_sectors; // Count of disk sectors in use by the inode, no including the inode structure nor dir entries linked.
    uint32_t flags;
    uint32_t os_specific_value_1;
    uint32_t direct_block_pointer[12];
    uint32_t single_indirect_block_pointer;
    uint32_t double_indirect_block_pointer;
    uint32_t triply_indirect_block_pointer;
    uint32_t gen_number;
    uint32_t extended_attribute_block;
    uint32_t high_size; // Upper 32 bits of size if file, otherwise directory ACL.
    uint32_t block_address_of_fragment;
    uint8_t  os_specific_value_2[12];

} __attribute__((packed)) __attribute__((aligned(256)));

struct directory_entry {
    uint32_t inode;
    uint16_t rec_len;        // Total length of the entry
    uint8_t  name_len_lsb_8; // Name length least-significant 8 bits
    uint8_t  type_indicator; // Type indicator (we don't allow > 256 chars in names.)
    char*    name;           // Name of the dir
} __attribute__((packed));

enum dir_entry_type_indicators {
    INDICATOR_TYPE_UNKNOWN_TYPE = 0,
    INDICATOR_TYPE_REGULAR_FILE,
    INDICATOR_TYPE_DIRECTORY,
    INDICATOR_TYPE_CHAR_DEVICE,
    INDICATOR_TYPE_BLOCK_DEVICE,
    INDICATOR_TYPE_FIFO,
    INDICATOR_TYPE_SOCKET,
    INDICATOR_TYPE_SOFT_SYM_LINK,
};

/**
 * Exits the program and returns control back to the OS.
 */
__attribute__((noreturn)) void exit(void);

/**
 * Invoke a generic system call.
 * 
 * This is here for convention and testing, one SHOULD use the named system calls.
 * 
 * @param sysno The number of the system call, defined in SYSTEM_CALL_ID
 * @param arg0 Argument for the system call.
 * @param arg1 Argument for the system call.
 * @param arg2 Argument for the system call.
 * 
 * @returns Integer value that MAY be null. Each typical system call may return different values.
 */
int syscall(int sysno, int arg0, int arg1, int arg2);

/**
 * Opens a file and returns a file descriptor for use.
 * 
 * @param path The unix style path to the file
 * @param access The requested permissions for the file
 * 
 * @returns A file descriptor number if >= 0, or an error.
 */
int32_t open(const char* path, int access);

/**
 * Reads in `size` bytes into the buffer. 
 * 
 * @param fd File descriptor for the file
 * @param buf Buffer pointer to read into
 * @param size Amount of bytes to read
 * 
 * @returns Amount of bytes read. If 0, EOF is reached.
 */
int64_t read(int32_t fd, void *buf, uint32_t size);

/**
 * Closes a given file.
 * 
 * @param fd File descriptor for the file
 */
void close(int32_t fd);


void putc(const char c); // Depreciated functions.
int getc(void); // Depreciated functions.




#endif
