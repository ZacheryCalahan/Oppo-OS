#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include <stddef.h>

// BPB reads are done in offsets, because who would EVER need to read an unaligned word. Pffftttttt...
// Standard BPB
#define BPB_JMP_BOOT                0x000
#define BPB_OEM_NAME                0x003
#define BPB_BYTES_PER_SECTOR        0x00B
#define BPB_SECTORS_PER_CLUSTER     0x00D
#define BPB_RESERVED_SECTORS        0x00E
#define BPB_NUM_FATS                0x010
#define BPB_NUM_ROOT_DIR            0x011
#define BPB_TOTAL_SECTORS_16        0x013
#define BPB_MEDIA                   0x015
#define BPB_FAT_SIZE_16             0x016
#define BPB_SECTORS_PER_TRACK       0x018
#define BPB_NUM_HEADS               0x01A
#define BPB_HIDDEN_SECTORS          0x01C
#define BPB_TOTAL_SECTORS_32        0x020

// FAT32 Extended BPB
#define BPB_FAT_SECTORS_32          0x024   // Sectors per FAT, in sector count.
#define BPB_EXT_FLAGS               0x028
#define BPB_FS_VERSION              0x02A
#define BPB_ROOT_CLUSTER            0x02C
#define BPB_FS_INFO                 0x030
#define BPB_BACKUP_BOOT_SECTOR      0x032
#define BPB_RESERVED                0x034
#define BPB_DRIVE_NUMBER            0x040
#define BPB_RESERVED1               0x041
#define BPB_BOOT_SIGNATURE          0x042
#define BPB_VOLUME_ID               0x043
#define BPB_VOLUME_LABEL            0x047
#define BPB_FS_TYPE                 0x052
#define BPB_PART_SIGNATURE          0x1FE

#define FAT32_SIGNATURE             0xAA55

struct mbr_partition_entry {
    uint8_t  status;          // 0x80 = bootable, 0x00 = non-bootable
    uint8_t  chs_first[3];    // Ignored
    uint8_t  type;            // 0x0B or 0x0C for FAT32
    uint8_t  chs_last[3];     // Ignored
    uint32_t lba_start;       // LBA where partition begins
    uint32_t num_sectors;     // Size of partition in sectors
}__attribute__((packed));

struct mbr {
    uint8_t boot_code[446];
    struct mbr_partition_entry partitions[4];    
    uint16_t signature; // 0xAA55
} __attribute__((packed));

#define FILE_ATTR_RO        0x01
#define FILE_ATTR_HIDDEN    0x02
#define FILE_ATTR_SYSTEM    0x04
#define FILE_ATTR_VOL_LABEL 0x08
#define FILE_ATTR_SUB_DIR   0x10
#define FILE_ATTR_ARCHIVE   0x20
#define FILE_ATTR_DEVICE    0x40 // Technically this is our OS extension, as I believe this is not typical of FAT32
#define FILE_ATTR_LFN_ENTRY 0x0F

#define DIR_ATTR_LFN_ENTRY  0x0F
#define DIR_ATTR_UNUSED     0xE5
#define DIR_ATTR_EOD        0x00

struct dir_entry {
    char name[11];
    uint8_t attr;
    uint8_t nt_reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_data;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed));

#define FAT_END_OF_CLUSTER_CHAIN 0xFFFFFFFF

enum FILE_ERR {
    SUCCESS = 0,
    FILE_NOT_FOUND = -1,        // File not found
    UNSUPPORTED_FEATURE = -2,   // Feature not supported (yet!)
    FILE_NOT_DIRECTORY = -3,    // Found a file, but expected a dir.
    PATH_INVALID = -4,          // Path supplied was not a valid path.
};

struct fsinfo {
    uint32_t lead_signature; // 0x41615252
    uint8_t reserved1[480];
    uint32_t struct_signature; // 0x61417272
    uint32_t free_count;
    uint32_t next_free;
    uint8_t reserved2[12];
    uint32_t trail_signature; // 0xAA550000
} __attribute__((packed));

static inline int is_eoc(uint32_t fat_data) {
    return fat_data >= 0x0FFFFFF8;
}

void init_fat32(void);
/**
* Get a file by its absolute path.
*
* @param path The absolute path of the file.
* @param file_size A buffer to hold the size of the returned file. Returns 0 on file not found.
*
* @return Pointer to the file if found, returns NULL on file not found.
*/
void *fat32_get_file_by_path(const char* path, uint32_t *file_size);

#endif