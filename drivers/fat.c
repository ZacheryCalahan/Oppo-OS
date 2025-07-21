#include "headers/fat.h"
#include "headers/blkio.h" // We'll just assume this device for now!
#include "../kernel/headers/kernel.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"

// Needed data for FAT parsing
uint64_t fat_begin_lba;             // First sector of the FAT
uint64_t cluster_begin_lba;         // First sector of the data region
uint64_t sectors_per_cluster;   
uint64_t root_dir_first_cluster;    // First cluster number of the root directory
uint64_t cluster_entries;           // Number of dir_entry in a single cluster
uint32_t *FAT;
uint32_t fat_entries;               // Size of the FAT in entries (for array access.)
uint32_t pages_per_cluster;         // Pages needed for a single clusterw

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

void *fat_get_cluster(uint32_t cluster) {
    // Obtain size to allocate in pages
    size_t cluster_pages = ((SECTOR_SIZE * sectors_per_cluster) + PAGE_SIZE - 1) / PAGE_SIZE;
    uintptr_t *data = kalloc(cluster_pages);

    if (!data) {
        PANIC("fat32: failed to allocate buffer for cluster");
    }

    // Compute start of the sectors that need to be read.
    uint32_t cluster_lba = cluster_begin_lba + (cluster - 2) * sectors_per_cluster;

    // Read in the sectors to the buffer
    for (uint32_t i = 0; i < sectors_per_cluster; i++) {
        uint8_t *sector_ptr = (uint8_t *)data + i * SECTOR_SIZE;
        read_write_disk(sector_ptr, cluster_lba + i, 0);
    }
    
    return data;
}

void init_fat32() {
    /* 
        The BPB is not naturally aligned, so certain reads WILL cause exceptions if words
        are not aligned on their respective barriers. (bytes_per_sector is a prime example!)
        We'll read all data using an array of u8s instead, just reading by offset.
        I imagine that the extra translation is inefficient, but it's intuitive!
        Technically we only need 512B, but a page is fine.
    */
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
    uint8_t num_fats = read_bpb_u8(sector0, BPB_NUM_FATS);
    if (num_fats != 2) {
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
    cluster_entries = (SECTOR_SIZE * sectors_per_cluster) / sizeof(struct dir_entry);
    pages_per_cluster = (sectors_per_cluster * SECTOR_SIZE) / PAGE_SIZE;

    // printf("fat32: Start sector of the FAT: %d\n", fat_begin_lba);
    // printf("fat32: Start sector of data: %d\n", cluster_begin_lba);
    // printf("fat32: Number of reserved sectors: %d\n", reserved_sectors);
    // printf("fat32: Number of FATs: %d\n", num_fats);
    // printf("fat32: Sectors in a cluster: %d\n", sectors_per_cluster);
    // printf("fat32: Cluster number of root directory: %d\n", root_dir_first_cluster);
    // printf("fat32: Number of dir_entries in a cluster: %d\n\n", cluster_entries);

    // Load FAT into memory
    fat_entries = (sectors_per_fat * SECTOR_SIZE) / sizeof(uint32_t);
    size_t fat_pages = ((sectors_per_fat * SECTOR_SIZE) + PAGE_SIZE - 1) / PAGE_SIZE;
    FAT = (uint32_t *) kalloc(fat_pages);

    if (!FAT) {
        PANIC("fat32: couldn't allocate memory for FAT");
    }

    for (uint32_t i = 0; i < sectors_per_fat; i++) {
        read_write_disk(((uint8_t *)FAT) + (i * SECTOR_SIZE), fat_begin_lba + i, 0); // Read into RAM
    }
    
    // printf("fat32: loaded FAT into memory (%u entries, %u bytes)\n", fat_entries, sectors_per_fat * SECTOR_SIZE);
}

void file_name_to_8_3(const char* path, char* buf) {
    // Determine what file name to search for in `cd`
    uint32_t path_index = 0; // Index into the path string
    char path_char = '\0';

    // Get the current path item to check for
    for (int i = 0; i < 11; i++) { // Build name char by char.
        path_char = path[path_index];

        // Determine if char indicates a file extension or directory and handle case.
        if (path_char == '/' || path_char == '\0') { // Is directory
            // Pad the rest of the name with ' '.
            while (i < 11) { 
                buf[i] = ' ';
                i++;
            }
            break;
        } else if (path_char == '.') { // Is file
            for (;i < 8; i++) { // Pad rest of file name, but not ext.
                buf[i] = ' ';
            }
            i--;
        } else {
            buf[i] = path_char;
        }
        
        path_index++; // Get next path char index
    }

    return;
}

void *fat32_get_file_by_path(const char* path, uint32_t *file_size) {

    // Start at root cluster
    uint32_t cluster_id = root_dir_first_cluster;
    void* cluster = fat_get_cluster(cluster_id);
    struct dir_entry *entries = (struct dir_entry *) cluster;
    int pathDelimIdx = 0;

    
    for (int dir_level = 0; dir_level < 3; dir_level++) { // 3 is arbitrary limit, remove later!
        // Get the current item of the path to search for
        char path_split[12];
        memset(path_split, 0, 12);
        for (int i = 0; i < 11; i++) {
            char c = path[pathDelimIdx];
            if (c == '/') { // Found directory
                pathDelimIdx++; // Ignore the '/'
                break;   
            } else if (c == '.') { // Found file, read in . and next 3 chars
                path_split[i] = c; i++; pathDelimIdx++;
                path_split[i] = path[pathDelimIdx]; i++; pathDelimIdx++;
                path_split[i] = path[pathDelimIdx]; i++; pathDelimIdx++;
                path_split[i] = path[pathDelimIdx]; i++; pathDelimIdx++;
            }
            path_split[i] = c;
            pathDelimIdx++; // Ready next char
        }
        char file_name[12];
        memset(file_name, 0, 12);
        file_name_to_8_3(path_split, file_name);
        printf("Looking for filename '%s'\n", file_name);

        // Read through cluster data
        for (int i = 0; i < cluster_entries; i++) {
            // Get the entry
            struct dir_entry entry = entries[i];        
            
            if (entry.name[0] == 0x00) {
                printf("No more entries.\n");
                file_size = 0;
                return NULL;
            }

            if (entry.name[0] == 0xE5) {
                printf("Deleted entry.\n");
                continue; // Deleted entry
            }

            // Read name
            char entry_name[12];
            memset(entry_name, 0, 12);
            memcpy(entry_name, entry.name, 11); // Copy entry to null terminated string.
            
            printf("Entry name: \"%s\" \n", entry_name);
                
            // Compare!
            if (strcmp(file_name, entry_name) == 0) {
                // We found the current path item!
                if (entry.attr & FILE_ATTR_SUB_DIR != 0) { // This is a subdir, continue down rabbit hole with next cluster
                    printf("Dir found!\n");
                    cluster_id = entry.first_cluster_low | (entry.first_cluster_high << 16);
                    kfree_order(cluster, order_for_pages(pages_per_cluster)); // Don't leak memory ya fool!
                    cluster = fat_get_cluster(cluster_id);
                    break;
                } else { // It's a file, assume we've reached the end!
                    printf("Returning the file...\n");
                    *file_size = entry.file_size;
                    uint32_t file_cluster = entry.first_cluster_low | (entry.first_cluster_high << 16);
                    
                    // Get the size of the file data
                    uint32_t file_cluster_count = 0; // We need to ensure the whole file is returned, so count the clusters!
                    uint32_t file_cluster_tmp = file_cluster;
                    while (!is_eoc(file_cluster_tmp) && file_cluster_tmp < fat_entries) {
                        file_cluster_count++;
                        file_cluster_tmp = FAT[file_cluster_tmp];
                    }
                    printf("File uses %d clusters.\n", file_cluster_count);

                    void* file_data = kalloc(((file_cluster_count * sectors_per_cluster * SECTOR_SIZE) + PAGE_SIZE - 1) / PAGE_SIZE);

                    // Fill the data and return
                    uint8_t* data_ptr = (uint8_t *) file_data;
                    file_cluster_tmp = file_cluster;

                    while (!is_eoc(file_cluster_tmp) && file_cluster_tmp < fat_entries) {
                        void* cluster_data = fat_get_cluster(file_cluster_tmp);
                        memcpy(data_ptr, cluster_data, sectors_per_cluster * SECTOR_SIZE);
                        kfree_order(cluster_data, order_for_pages(pages_per_cluster));
                        data_ptr += sectors_per_cluster * SECTOR_SIZE;
                        file_cluster_tmp = FAT[file_cluster_tmp];
                    }

                    return file_data;
                }
            }
        }

        // Traverse to the next cluster if file/dir not found in this cluster
        if (is_eoc(FAT[cluster_id])) {
            // No next cluster, so file does not exist!
            *file_size = 0;
            return NULL;
        }

        uint32_t new_cluster_id = FAT[cluster_id];
        cluster_id = new_cluster_id;
        cluster = (uint32_t *) fat_get_cluster(cluster_id);
    }

    // How did we get here?
    
}




