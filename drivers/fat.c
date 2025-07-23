#include "headers/fat.h"
#include "headers/blkio.h" // We'll just assume this device for now!
#include "../kernel/headers/kernel.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"

// Needed data for FAT parsing
uint64_t fat_begin_lba;             // First sector of the FAT
uint64_t cluster_begin_lba;         // First sector of the data region
uint64_t sectors_per_cluster;
uint64_t bytes_per_cluster;
uint64_t root_dir_first_cluster;    // First cluster number of the root directory
uint64_t cluster_entries;           // Number of dir_entry in a single cluster
uint32_t *FAT;
uint32_t fat_entries;               // Size of the FAT in entries (for array access.)
uint32_t pages_per_cluster;         // Pages needed for a single cluster

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

uint32_t convert_bytes_to_clusters(uint64_t bytes) {
    return (bytes + (sectors_per_cluster * SECTOR_SIZE) - 1) / (sectors_per_cluster * SECTOR_SIZE);
}

void path_to_directory_tokens(const char* path, char tokens[64][12], uint32_t *argc) {
    uint32_t path_len = strlen(path);
	*argc = 0; // Args counter (number of tokens)
	memset(tokens, 0, 64 * 12); // Clear the tokens string array

	char current_token[12];
	int current_token_index = 0;
	memset(current_token, 0, 12); // Ensure string is empty.

	for (int i = 0; i < path_len; i++) {
		char current_char = path[i];

		if (current_char == '/') { // Directory, end of token.
			strcpy(tokens[*argc], current_token); // Write the current token
			current_token_index = 0;
			(*argc)++;
			memset(current_token, 0, 12); // Clear the current token.
			continue;
		} else if (i == path_len - 1) { // End of the path, submit and finish
			current_token[current_token_index] = current_char;
			strcpy(tokens[*argc], current_token);
			(*argc)++;
			return;
		}

		current_token[current_token_index] = current_char; // Just write the char, nothing fancy.
		current_token_index++;
	}
}

uint32_t write_to_cluster(void* data, uint32_t cluster) {
    uint32_t cluster_lba = cluster_begin_lba + (cluster - 2) * sectors_per_cluster;

    // Read in the sectors to the buffer
    for (uint32_t i = 0; i < sectors_per_cluster; i++) {
        uint8_t *sector_ptr = (uint8_t *)data + i * SECTOR_SIZE;
        read_write_disk(sector_ptr, cluster_lba + i, 1);
    }
}

uint32_t find_free_cluster() {
    for (int i = 2; i < fat_entries; i++) {
        if (FAT[i] == 0) {
            return i;
        }
    }

    return 0;
}

uint32_t update_fat_chain(uint64_t size_bytes, uint32_t start_cluster_id) {
    uint32_t clusters_needed = convert_bytes_to_clusters(size_bytes);
    uint32_t current_cluster_id;
    uint32_t prev_cluster_id = 0;
    uint32_t clusters_used = 0;
    

    // Count currently used clusters
    while (!is_eoc(FAT[current_cluster_id]) && clusters_used < clusters_needed) {
        clusters_used++;
        prev_cluster_id = current_cluster_id;
        current_cluster_id = FAT[current_cluster_id]; // Get next cluster
    }

    // Update the chain
    if (clusters_used == clusters_needed) {
        // Shrink the chain
        if (!is_eoc(FAT[current_cluster_id])) {
            uint32_t to_free = current_cluster_id;
            FAT[prev_cluster_id] = FAT_END_OF_CLUSTER_CHAIN;
            // Free remaining clusters
            while (!is_eoc(FAT[to_free])) {
                uint32_t next = FAT[to_free];
                FAT[to_free] = 0;
                to_free = next;
            }
        }
    } else {
        // Grow the chain
        uint32_t last = prev_cluster_id;
        while (clusters_used < clusters_needed) {
            // Find a free cluster
            uint32_t new_cluster = find_free_cluster();
            if (new_cluster == 0) {
                PANIC("Out of clusters!");
            }

            FAT[last] = new_cluster;
            last = new_cluster;
            clusters_used++;
        }

        FAT[last] = FAT_END_OF_CLUSTER_CHAIN;
    }

    return clusters_used;
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
    bytes_per_cluster = (sectors_per_cluster * SECTOR_SIZE);

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

    // Get path info
    char file_names[64][12];
    uint32_t argc;
    path_to_directory_tokens(path, file_names, &argc);
    
    // Traverse the directories until we find the file in the path
    int dir_level = 0; // How many directories we've entered
    int dir_found = 0; // If we find a dir, skip searching for next cluster in the chain.
    while (dir_level < argc) {
        // Get the current item of the path to search for
        char file_name[12];
        memset(file_name, 0, 12);
        file_name_to_8_3(file_names[dir_level], file_name);

        // Read through entries in the cluster
        for (int entry_number = 0; entry_number < cluster_entries; entry_number++) {
            // Get the entry
            struct dir_entry entry = entries[entry_number];        
            
            if (entry.name[0] == 0x00) {
                // printf("No more entries.\n");
                file_size = 0;
                return NULL;
            }
            if (entry.name[0] == 0xE5) {
                // printf("Deleted entry.\n");
                continue; // Deleted entry
            }

            // Read name
            char entry_name[12];
            memset(entry_name, 0, 12);
            memcpy(entry_name, entry.name, 11); // Copy entry to null terminated string.
                
            // Found the dir/file, now handle entering subdir or return file.
            if (strcmp(file_name, entry_name) == 0) {
                // printf("Entry attr: 0x%x\n", entry.attr);
                if ((entry.attr & FILE_ATTR_LFN_ENTRY) != 0) {
                    PANIC("LFN Entries are not currently supported.");
                }

                if ((entry.attr & FILE_ATTR_SUB_DIR) != 0) { // This is a subdir, continue down rabbit hole with next cluster
                    printf("Dir found!\n");
                    cluster_id = entry.first_cluster_low | (entry.first_cluster_high << 16);
                    kfree_order(cluster, order_for_pages(pages_per_cluster)); // Don't leak memory ya fool!
                    cluster = fat_get_cluster(cluster_id);
                    dir_level++;
                    dir_found = 1; // Note to not jump to next cluster in the chain
                    break;
                } else { // It's a file!
                    printf("dir: %d, argc: %d\n", dir_level, argc);
                    if (dir_level != (argc - 1)) { // Ensure we're at the end of the path, don't return files in the middle of the path.
                        *file_size = 0;
                        return NULL;
                    }

                    // Get the size of the file data
                    *file_size = entry.file_size;
                    uint32_t file_cluster = entry.first_cluster_low | (entry.first_cluster_high << 16);
                    
                    // Get the size of the file data
                    uint32_t file_cluster_count = 0; // We need to ensure the whole file is returned, so count the clusters!
                    uint32_t file_cluster_tmp = file_cluster;
                    while (!is_eoc(file_cluster_tmp) && file_cluster_tmp < fat_entries) {
                        file_cluster_count++;
                        file_cluster_tmp = FAT[file_cluster_tmp];
                    }
                    // printf("File uses %d clusters.\n", file_cluster_count);

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

        if (!dir_found) {
            // Traverse to the next cluster if file/dir not found in this cluster
            if (is_eoc(FAT[cluster_id])) {
                // No next cluster, so file does not exist!
                printf("End of cluster!\n");
                *file_size = 0;
                return NULL;
            }

            cluster_id = FAT[cluster_id];
            cluster = (uint32_t *) fat_get_cluster(cluster_id);
        }
        dir_found = 0; // Reset, since we've either entered a new dir, or it was already 0.
    }    
}

enum FILE_ERR fat32_write_file_by_path(const char* path, void* file_data, uint64_t file_size, uint8_t flags) {
    // Start at root cluster
    uint32_t cluster_id = root_dir_first_cluster;
    void* cluster = fat_get_cluster(cluster_id);
    struct dir_entry *entries = (struct dir_entry *) cluster;
    struct dir_entry last_deleted_entry;

    // Get path info
    char file_names[64][12];
    uint32_t argc;
    path_to_directory_tokens(path, file_names, &argc);
    
    // Traverse the directories until we find the file in the path
    int dir_level = 0; // How many directories we've entered
    int dir_found = 0; // If we find a dir, skip searching for next cluster in the chain.
    while (dir_level < argc) {
        // Get the current item of the path to search for
        char file_name[12];
        memset(file_name, 0, 12);
        file_name_to_8_3(file_names[dir_level], file_name);

        // Read through entries in the cluster
        for (int entry_number = 0; entry_number < cluster_entries; entry_number++) {
            // Get the entry
            struct dir_entry entry = entries[entry_number];        
            
            if (entry.name[0] == 0x00) {
                // printf("No more entries.\n");
                file_size = 0;
                return NULL;
            }
            if (entry.name[0] == 0xE5) {
                last_deleted_entry = entry; // We may use this for writing later!
                continue; // Deleted entry
            }

            // Read name
            char entry_name[12];
            memset(entry_name, 0, 12);
            memcpy(entry_name, entry.name, 11); // Copy entry to null terminated string.
                
            // Found the dir/file, now handle entering subdir or return file.
            if (strcmp(file_name, entry_name) == 0) {
                // printf("Entry attr: 0x%x\n", entry.attr);
                if ((entry.attr & FILE_ATTR_LFN_ENTRY) != 0) {
                    PANIC("LFN Entries are not currently supported.");
                }

                if ((entry.attr & FILE_ATTR_SUB_DIR) != 0) { // This is a subdir, continue down rabbit hole with next cluster
                    printf("Dir found!\n");
                    cluster_id = entry.first_cluster_low | (entry.first_cluster_high << 16);
                    kfree_order(cluster, order_for_pages(pages_per_cluster)); // Don't leak memory ya fool!
                    cluster = fat_get_cluster(cluster_id);
                    dir_level++;
                    dir_found = 1; // Note to not jump to next cluster in the chain
                    break;
                } else { // It's a file!
                    printf("dir: %d, argc: %d\n", dir_level, argc);
                    if (dir_level != (argc - 1)) { // Ensure we're at the end of the path, don't return files in the middle of the path.
                        return NULL;
                    }

                    // At end of the path, with a matching existing file. Shrink/extend FAT cluster entries, then overwrite file.
                    PANIC("NOT IMPLEMENTED FILE OVERWRITE");
                }
            }
        }

        if (!dir_found) {
            // Traverse to the next cluster if file/dir not found in this cluster
            if (is_eoc(FAT[cluster_id])) {
                // No next cluster, so file does not exist!
                if ((argc - 1) != dir_level) {
                    return FILE_NOT_FOUND;
                }
                
                // File doesn't exist, and at the end of path in the correct directory. Make FAT cluster entries, then write file.
                PANIC("NOT IMPLEMENTED NEW FILE WRITING");
            }

            cluster_id = FAT[cluster_id];
            cluster = (uint32_t *) fat_get_cluster(cluster_id);
        }
        dir_found = 0; // Since we've either entered a new dir, or it was already 0.
    }    
}




