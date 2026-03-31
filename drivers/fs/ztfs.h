#ifndef ZTFS_H
#define ZTFS_H

#include <stdint.h>
#include <stddef.h>

#define ZTFS_VERSION_MAJOR 1
#define ZTFS_VERSION_MINOR 0

#define ZTFS_BLUEPRINT_ADDR 512
#define ZTFS_BGDT_START_BADDR 2 // Block address where BGDT starts. This does not change.
#define ZTFS_MAGIC_SIGNATURE 0x5346545AUL

typedef uint32_t baddr_t;

enum ENTRY_TYPE {
    NONE = 0,
    ENTRY_DIRECTORY = 1,
    ENTRY_FILE = 2,
    ENTRY_REF = 3,          // Link
    ENTRY_CHAR = 4,         // Character device
    ENTRY_BLOCK = 5,        // Block device
    ENTRY_SOCKET = 6        // Net socket
};

enum PERMISSIONS {
    SUPER_READ = 1, // Super is locked to the kernel
    SUPER_WRITE = 2,
    SUPER_RW = SUPER_READ | SUPER_WRITE,
    SUPER_EXECUTE = 4,
    SUPER_RWX = SUPER_RW | SUPER_EXECUTE,
    USER_READ = 8,  // User allows users to access this file
    ALL_READ = SUPER_READ | USER_READ,
    USER_WRITE = 16,
    ALL_WRITE = SUPER_WRITE | USER_WRITE,
    USER_RW = USER_READ | USER_WRITE,
    USER_EXECUTE = 32,
    ALL_EXECUTE = SUPER_EXECUTE | USER_EXECUTE,
    USER_RWX = USER_RW | USER_EXECUTE,
    ALL_RW = SUPER_RW | USER_RW,
    ALL_RWX = SUPER_RWX | USER_RWX,
    APPEND_ONLY = 64,
    PERM_RESERVED = 128,
};

struct ztfs_blueprint {
    uint64_t signature;                 // Signature is required to mark this is a ZTFS filesystem
    uint16_t version_major;             // Major version number
    uint16_t version_minor;             // Minor version number
    uint32_t block_size;                // Size of a block in bytes
    uint64_t size_bytes;                // Size of the filesystem in bytes
    uint64_t free_blocks;               // Number of free blocks available for use
    uint32_t baddr_root_entry;          // Block address of the root directory entry
    uint32_t block_group_table_size;    // Blocks in use by the block group table
    uint32_t baddr_bitmap_start;        // Start address of the preallocated bitmaps, for convience
    uint32_t block_group_size;          // Size of a block group in blocks
    uint32_t baddr_first_block_group;   // Block address for the start of the first block group
    uint32_t block_group_count;         // Number of block groups in file system
};

struct ztfs_block_group_descriptor {
    uint32_t baddr_block_bitmap;        // Block address for the used block bitmap
    uint32_t baddr_first_entry;         // Block address for the first entry, for convience.
    uint32_t free_blocks;               // Number of free blocks available for use in this block group
    uint16_t num_entries;               // Number of entries this block group has, for convience
    uint16_t reserved;                  // Padding for version 1.0
};

struct ztfs_entry {
    char name[64];                          // Name of the entry, \0 terminated.
    union {
        uint64_t file_size_bytes;           // If file, this is the size in bytes
        uint64_t entry_count;               // If dir, this is the count of the subentries
        uint64_t ref_target_entry_index;    // If ref, this is the index into the entry block to referred entry
    };
    uint32_t size_blocks;                   // Number of blocks the data is using
    uint8_t entry_type;                     // Type of entry, ie. directory, file, etc.
    uint8_t permissions;                    // Permissions of the entry
    union {
        uint32_t baddr_indirect_block;      // Block address of the indirect block
        uint32_t ref_target_block;          // Block address of the referred entry
    };

    uint32_t baddr_double_indir_block;      // If requiring more than the single indirect, this points to a block of indirect baddrs.
    uint16_t ref_count;                     // Number of times this is referenced
    
    // Entry location
    uint8_t entry_idx;                      // This entry's index into its parent's data block entry array. (for reference use)
    uint8_t data_block_idx;                 // This entry's index into its parent's indirect block. (for reference use)
    baddr_t entry_baddr;                    // This entry's block address (for reference use)
};

// High level interface


// Driver specific functions
/**
 * @brief Read `size` bytes `n` times starting from absolute address `offset`.
 * 
 *
 * @param buffer Buffer for data read in, must be `size * n` bytes long.
 * @param size Bytes to read in
 * @param n Times to read in `size` bytes
 * @param offset Absolute address on disk to read
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_read(void *buffer, size_t size, size_t n, size_t offset);

/**
 * Write `size` bytes `n` times starting from absolute address `offset`.
 *
 * @param buffer Buffer of data to write, must be `size` bytes long.
 * @param size Bytes to write
 * @param n Times to write `size` bytes
 * @param offset Absolute address on disk to write to 
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_write(void *buffer, size_t size, size_t n, size_t offset);

/**
 * @brief Read block group `bg_num` into `desc`.
 * 
 *
 * @param desc Buffer to read in to
 * @param bg_num Block group number of the descriptor
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_read_bgd(struct ztfs_block_group_descriptor *desc, uint32_t bg_num);

/**
 * @brief Write `desc` into block group `bg_num`.
 * 
 *
 * @param desc Buffer of data to write
 * @param bg_num Block group number of the descriptor
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_write_bgd(struct ztfs_block_group_descriptor *desc, uint32_t bg_num);

/**
 * @brief Read in `blueprint`
 * 
 *
 * @param blueprint Buffer to read data in to
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_read_blueprint(struct ztfs_blueprint *blueprint);

/**
 * @brief Write to `blueprint`
 * 
 *
 * @param blueprint Buffer holding `blueprint` data to write
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_write_blueprint(struct ztfs_blueprint *blueprint);

/**
 * @brief Read in a full block bitmap into `bitmap_block` from group `bg_num`.
 * 
 *
 * @param bitmap_block Buffer to hold data read in
 * @param bg_num Number of the block group to get bitmap from
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_read_bitmap(void* bitmap_block, uint32_t bg_num);

/**
 * @brief Mark `baddr_block` block used.
 * 
 *
 * @param baddr_block Block address to mark as used
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_mark_block_used_bitmap(baddr_t baddr_block);

/**
 * @brief Mark `baddr_block` block as a free block.
 * 
 *
 * @param baddr_block Block address to mark as free
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_free_block_bitmap(baddr_t baddr_block);

/**
 * @brief Find a free block.
 * 
 * @returns Block address of a free block, or 0 if not found.
 */
baddr_t ztfs_find_unused_block(void);

/**
 * @brief Find an `entry` from absolute `path`.
 * 
 *
 * @param entry Buffer to read in entry found
 * @param path Absolute path to entry
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_find_entry_via_path(struct ztfs_entry *entry, char *path);

/**
 * @brief Find a free entry in `parent` and mark location in `data_block_idx` and `entry_idx`.
 * 
 *
 * @param parent Parent entry to search for free entry
 * @param data_block_idx Buffer for index into indirect block the free entry is in
 * @param entry_idx Buffer for index into entry array the free entry is in
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_find_free_entry(struct ztfs_entry *parent, uint32_t *data_block_idx, uint32_t *entry_idx);

/**
 * @brief Insert `child` entry into `parent` entry.
 * 
 *
 * @param parent Parent entry
 * @param child Entry to insert into parent
 * 
 * @returns 0 on success, and -1 on failure.
 */
int ztfs_place_entry(struct ztfs_entry *parent, struct ztfs_entry *child);

/**
 * @brief Find the block group that `baddr_block` resides in.
 * 
 *
 * @param baddr_block Block address to find the block group of
 * 
 * @returns -1 on failure, or number of the block group on success.
 */
uint32_t ztfs_find_block_group_from_baddr(baddr_t baddr_block);


#endif