#include "headers/fat.h"
#include "headers/blkio.h" // We'll just assume this device for now!
#include "../kernel/headers/kernel.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"

// Needed data for FAT parsing
uint64_t fat_begin_lba;
uint64_t cluster_begin_lba;
uint64_t sectors_per_cluster;
uint64_t root_dir_first_cluster;

uint64_t read_bpb_u64(uint8_t *bpb, size_t offset) {
    return ((uint64_t)bpb[offset]) |
           ((uint64_t)bpb[offset + 1] << 8) |
           ((uint64_t)bpb[offset + 2] << 16) |
           ((uint64_t)bpb[offset + 3] << 24) |
           ((uint64_t)bpb[offset + 4] << 32) |
           ((uint64_t)bpb[offset + 5] << 40) |
           ((uint64_t)bpb[offset + 6] << 48) |
           ((uint64_t)bpb[offset + 7] << 56);
}

uint32_t read_bpb_u32(uint8_t *bpb, size_t offset) {
    return ((uint32_t)bpb[offset]) |
           ((uint32_t)bpb[offset + 1] << 8) |
           ((uint32_t)bpb[offset + 2] << 16) |
           ((uint32_t)bpb[offset + 3] << 24);
}

uint16_t read_bpb_u16(uint8_t *bpb, size_t offset) {
    return ((uint16_t)bpb[offset]) |
           ((uint16_t)bpb[offset + 1] << 8);
}

uint8_t read_bpb_u8(uint8_t *bpb, size_t offset) {
    return (uint8_t) bpb[offset]; // Yeah yeah this is for readability, the overhead is small enough for this os.
}

void init_fat32() {
    // The BPB is not naturally aligned, so certain reads WILL cause exceptions if words
    // are not aligned on their respective barriers. (bytes_per_sector is a prime example!)
    // We'll read all data using an array of u8s instead, just reading by offset.
    // I imagine that the extra translation is inefficient, but it's intuitive!
    // Technically we only need 512B, but a page is fine.
    uint8_t *sector0 = kalloc(1); // BPB data pointer
    read_write_disk(sector0, 0, 0);

    // Check signature
    if (read_bpb_u16(sector0, BPB_PART_SIGNATURE) != FAT32_SIGNATURE) {
        PANIC("fat32: not fat32 partition %x", read_bpb_u16(sector0, BPB_PART_SIGNATURE));
    }
    // Ensure sector size is correct
    if (read_bpb_u16(sector0, BPB_BYTES_PER_SECTOR) != SECTOR_SIZE) {
        PANIC("fat32: invalid sector size %x", read_bpb_u16(sector0, BPB_BYTES_PER_SECTOR));
    }
    // Get (and check!) number of FATs
    uint8_t num_fats;
    if (num_fats = read_bpb_u8(sector0, BPB_NUM_FATS) != 2) {
        PANIC("fat32: unsupported FAT count %x", num_fats);
    }
    
    // Get number of reserved sectors
    uint16_t reserved_sectors = read_bpb_u16(sector0, BPB_RESERVED_SECTORS);

    // Get sectors per FAT
    uint32_t sectors_per_fat = read_bpb_u32(sector0, BPB_FAT_SECTORS_32);
    
    // Now calculate all needed info for the FAT parsing! May need tweaking, I'm assuming LBA_begin is 0.
    fat_begin_lba = reserved_sectors;
    cluster_begin_lba = reserved_sectors + (num_fats * sectors_per_fat);
    sectors_per_cluster = read_bpb_u8(sector0, BPB_SECTORS_PER_CLUSTER);
    root_dir_first_cluster = read_bpb_u32(sector0, BPB_ROOT_CLUSTER);
}

void fat_walk() {
    
}


