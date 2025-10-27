#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

// Supported features by this current implementation.
#define SUPPORTS_MAJOR_VERSION          1
#define SUPPORTS_MINOR_VERSION          0

#define SUPPORTS_COMPRESSION            0
#define SUPPORTS_DIR_ENTRY_TYPES        1
#define SUPPORTS_JOURNAL_DEVICE         0
#define SUPPORTS_PREALLOC_FOR_DIRS      0
#define SUPPORTS_AFS_SRV_INODES         0
#define SUPPORTS_JOURNAL                0
#define SUPPORTS_EXT_INODE_ATTR         0
#define SUPPORTS_FS_RESIZABLE           0
#define SUPPORTS_DIR_HASH_INDEX         0
#define SUPPORTS_SPARSE_SB_DT           0
#define SUPPORTS_64BIT_FILE_SIZE        0
#define SUPPORTS_DIR_CONTENTS_BIN_TREE  0

enum file_system_state {
    CLEAN_FILE_SYSTEM = 1,
    ERROR_FILE_SYSTEM = 2
};

enum error_handle_methods {
    IGNORE = 1,
    READ_ONLY = 2,
    PANIC = 3,
};

enum creator_os_ids {
    LINUX,
    GNU_HURD,
    MASIX,
    FREE_BSD,
    OTHER_LITES,
};

#define EXT2_SIGNATURE 0xEF53

struct extended_s_block {
    uint32_t first_non_reserved_inode;              // First non-reserved inode in file system.
    uint16_t size_inode_bytes;                      // Size of each inode structure in bytes.
    uint16_t current_block_group;                   // Block group that this superblock is part of (if backup copy).
    uint32_t optional_features_present;             // Non-required features, but are available for support.
    uint32_t required_features_present;             // Required features to read or write.
    uint32_t rw_required_features;                  // Required features to write, if unsupported allows reading only.
    uint8_t  file_system_id[16];                    // File system ID.
    char     volume_name[16];                       // Volume name in null terminated string.
    char     last_mounted_path_volume[64];          // Path volume that was last mounted to, null terminated string.
    uint32_t compression_algorithms_used;           // Compression algorithms used.
    uint8_t  number_blocks_to_preallocate_files;    // Number of blocks to preallocate for files.
    uint8_t  number_blocks_to_preallocate_dirs;     // Number of blocks to preallocate for directories.
    uint16_t reserved1;                             // Unused.
    uint8_t  journal_id[16];                        // Journal ID.
    uint32_t journal_inode;                         // Journal inode.
    uint32_t journal_device;                        // Journal device.
    uint32_t head_orphan_inode_list;                // Head of orphan inode list.
    uint8_t  reserved2[787];                        // Unused.
} __attribute__((packed));

struct s_block {
    uint32_t inode_count;               // Total number of inodes in file system.
    uint32_t block_count;               // Total number of blocks in file system.
    uint32_t block_reserved_su;         // Number of blocks reserved for superuser.
    uint32_t unallocated_block_count;   // Total number of unallocated blocks.
    uint32_t unallocated_inode_count;   // Total number of unallocated inodes.
    uint32_t superblock_block_number;   // Block number of the block containing the superblock.
    uint32_t block_size_shift;          // Block size where (block_size = 1024 << block_size_shift)
    uint32_t fragment_size_shift;       // Fragment size where (fragment_size = 1024 << fragment_size_shift)
    uint32_t blocks_in_block_group;     // Number of blocks in each block group.
    uint32_t fragments_in_block_group;  // Number of fragments in each block group.
    uint32_t inodes_in_block_group;     // Number of inodes in each block group.
    uint32_t last_mount_time;           // Last mount time (POSIX time).
    uint32_t last_written_time;         // Last written time (POSIX time).
    uint16_t mount_count_fsck;          // Number of times the volume has been mounted since last fsck.
    uint16_t mount_count_until_fsck;    // Number of mounts allowed before a fsck must be done.
    uint16_t signature;                 // EXT2 signature (0xEF53).
    uint16_t file_system_state;         // State of filesystem. See: `enum file_system_state`.
    uint16_t error_action;              // What to do when encountering an error. See: `enum error_handle_methods`.
    uint16_t minor_version;             // Minor portion of the version.
    uint32_t time_since_last_fsck;      // Time since last fsck (POSIX time).
    uint32_t interval_forced_fsck;      // Interval between forced fsck (POSIX time).
    uint32_t os_id;                     // OS ID from which the filesystem on this volume was created.
    uint32_t major_version;             // Major portion of the version.
    uint16_t user_id_reserved_blocks;   // User ID that can use reserved blocks.
    uint16_t group_id_reserved_blocks;  // Group ID that can use reserved blocks.
    struct extended_s_block esb;        // If major version is >= 1, extended superblock is here.
} __attribute__((packed));

#define OPT_FEAT_PREALLOCATE_BLKS_IN_DIR    (1 << 0)
#define OPT_FEAT_AFS_SERVER_INODES_EXIST    (1 << 1)
#define OPT_FEAT_FS_HAS_JOURNAL             (1 << 2)
#define OPT_FEAT_INODE_EXT_ATTR             (1 << 3)
#define OPT_FEAT_FS_RESIZEABLE              (1 << 4)
#define OPT_FEAT_FS_DIR_HASH_INDEX          (1 << 5)
#define REQ_FEAT_COMPRESSION                (1 << 0)
#define REQ_FEAT_DIR_ENTRY_TYPE_FIELD       (1 << 1)
#define REQ_FEAT_FS_REPLAY_JOURNAL          (1 << 2)
#define REQ_FEAT_FS_JOURNAL_DEVICE          (1 << 3)
#define RO_FEAT_SPARSE_SBLOCK_GROUP_DES_T   (1 << 0)
#define RO_FEAT_64BIT_FILE_SIZE             (1 << 1)
#define RO_FEAT_DIR_CONTENTS_BINARY_TREE    (1 << 2)

void init_ext2(void);

#endif