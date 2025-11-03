#include "ext2.h"
#include "../virtio/virtio_blkio.h"
#include "../../kernel/kernel.h"
#include "../../klib/stdlib.h"
#include "../../klib/stdio.h"

uint32_t NUM_INDIRECT_BLOCKS; // Number of block pointers in a single direct block

uint32_t ext2_block_size;
uint32_t ext2_inode_count;
uint32_t ext2_block_count;
uint32_t ext2_blocks_per_bgroup;
uint32_t ext2_inodes_per_bgroup;
uint32_t ext2_inode_size_bytes;
int is_read_only;

void init_ext2() {
    struct s_block *superblock = (struct s_block *) kalloc(1);
    read_write_disk(superblock, 2, 0); // Read in the super block (1024 bytes)
    read_write_disk(superblock + SECTOR_SIZE, 3, 0);
    if (superblock->signature != EXT2_SIGNATURE) {
        PANIC("EXT2: Superblock not found.");
    }

    // Extracts
    ext2_block_size = (1024 << superblock->block_size_shift);
    printf("EXT2: Block size: %d\n", ext2_block_size);
    ext2_inode_count = superblock->inode_count;
    printf("EXT2: Inode Count: %d\n", ext2_inode_count);
    ext2_block_count = superblock->block_count;
    printf("EXT2: Block Count: %d\n", ext2_block_count);
    ext2_blocks_per_bgroup = superblock->blocks_in_block_group;
    printf("EXT2: Blocks per Block Group: %d\n", ext2_blocks_per_bgroup);
    ext2_inodes_per_bgroup = superblock->inodes_in_block_group;
    printf("EXT2: Inodes per Block Group: %d\n", ext2_inodes_per_bgroup);
    ext2_inode_size_bytes = superblock->esb.size_inode_bytes;
    printf("EXT2: Inode Size: %d\n", ext2_inode_size_bytes);

    NUM_INDIRECT_BLOCKS = ext2_block_size / sizeof(uint32_t);

    // Check required features (if major >= 1.)
    if (superblock->major_version >= 1) {
        printf("EXT2: Version %d.%d\n", superblock->major_version, superblock->minor_version);
        
        // Read write features (Required Features)
        uint32_t req_feats = superblock->esb.required_features_present;
        if ((req_feats & REQ_FEAT_COMPRESSION) && !SUPPORTS_COMPRESSION) {
            PANIC("EXT2: Required feature not supported: compression.");          
        }   
        if ((req_feats & REQ_FEAT_DIR_ENTRY_TYPE_FIELD) && !SUPPORTS_DIR_ENTRY_TYPES) {
            PANIC("EXT2: Required feature not supported: dir entry type field.");
        }
        if ((req_feats & REQ_FEAT_FS_REPLAY_JOURNAL)) {
            PANIC("EXT2: Required feature not supported: replay journal");
        }
        if ((req_feats & REQ_FEAT_FS_JOURNAL_DEVICE) && !SUPPORTS_JOURNAL_DEVICE) {
            PANIC("EXT2: Required feature not supported: journal device");
        }
            
        // Write features
        uint32_t req_w_feats = superblock->esb.rw_required_features;
        if ((req_w_feats & RO_FEAT_SPARSE_SBLOCK_GROUP_DES_T) && !SUPPORTS_SPARSE_SB_DT) {
            printf("EXT2: Feature `Sparse superblocks and group descriptor tables` found but not supported.\n");
            is_read_only = 1;
        } 
        if ((req_w_feats & RO_FEAT_64BIT_FILE_SIZE) && !SUPPORTS_64BIT_FILE_SIZE) {
            printf("EXT2: Feature `64-bit file sizes` found but not supported.\n");
            is_read_only = 1;
        }
        if ((req_w_feats & RO_FEAT_DIR_CONTENTS_BINARY_TREE) && !SUPPORTS_DIR_CONTENTS_BIN_TREE ) {
            printf("EXT2: Feature `Directory contents stored in binary tree` found but not supported.\n");
            is_read_only = 1;
        }
        if (is_read_only) {
            printf("EXT2: File system mounted as read-only!\n");
        }
        // Optional features (we don't use them, don't even check them.)

        kfree_size(superblock, PAGE_SIZE);
    }
}

void* read_block(uint32_t block_num) {
    void* buffer = kalloc(1); // Allocate one page, the size of a block.
    
    // Calculate where to read on disk and how far
    uint32_t sector_num = (block_num * PAGE_SIZE) / SECTOR_SIZE;
    uint32_t sector_count = PAGE_SIZE / SECTOR_SIZE;

    for (int i = 0; i < sector_count; i++) {
        read_write_disk(buffer + (i * SECTOR_SIZE), sector_num + i, 0);
    }
    
    return buffer;
}

struct inode* get_root_inode() {
    // Get the initial BGDT from the first block group
    struct block_group_descriptor_table *bgdt = (struct block_group_descriptor_table *) read_block(1);

    // Retrieve the inode block start address, and pull the data from the address.
    uint32_t inode_table_block = bgdt->inode_block_start_block_addr;
    printf("Inode table start block: %d\n", inode_table_block);

    // Read the root inode from the table
    struct inode *inode_table = read_block(inode_table_block);

    // Ensure that this is a directory
    uint16_t type = inode_table->permissions;
    if (type & INODE_TYPE_DIRECTORY == 0) {
        // This is not a directory!
        return NULL;
    }
    
    // Allocate room for the root inode to return.
    struct inode *root_inode = kalloc(1);
    struct inode *src_inode = &inode_table[1]; // Root node is always inode number 2. (inodes are indexed at 1, so 2nd member)
    memcpy(root_inode, src_inode, ext2_inode_size_bytes);

    // Free used resources.
    kfree_size(bgdt, PAGE_SIZE);
    kfree_size(inode_table, PAGE_SIZE);
    
    return root_inode;
}

uint32_t get_block_address(struct inode *i_node, uint32_t block_index) {
    if (block_index < 12) {
        // Direct block pointer
        return i_node->direct_block_pointer[block_index];
    } else if (block_index < 12 + NUM_INDIRECT_BLOCKS) {
        // Single indirect block pointer
        uint32_t *indirect_block = read_block(i_node->single_indirect_block_pointer);
        uint32_t block_ptr = indirect_block[block_index - 12];
        kfree_size(indirect_block, PAGE_SIZE); // Don't leak memory ya fool!
        return block_ptr;
    } else if (block_index < 12 + NUM_INDIRECT_BLOCKS + NUM_INDIRECT_BLOCKS * NUM_INDIRECT_BLOCKS) {
        // Double indirect block pointer
        uint32_t double_offset = block_index - 12 - NUM_INDIRECT_BLOCKS;
        uint32_t double_block_offset = double_offset / NUM_INDIRECT_BLOCKS;
        uint32_t single_block_offset = double_offset % NUM_INDIRECT_BLOCKS;

        uint32_t *double_indirect = read_block(i_node->double_indirect_block_pointer);
        uint32_t *indirect_block = read_block(double_indirect[double_block_offset]);
        uint32_t block_ptr = indirect_block[single_block_offset];
        kfree_size(double_indirect, PAGE_SIZE);
        kfree_size(indirect_block, PAGE_SIZE);
        return block_ptr;
    } else {
        // Triply indirect block pointer
        uint32_t triple_offset = block_index - 12 - NUM_INDIRECT_BLOCKS - NUM_INDIRECT_BLOCKS * NUM_INDIRECT_BLOCKS * NUM_INDIRECT_BLOCKS;
        uint32_t triple_block_offset = triple_offset / (NUM_INDIRECT_BLOCKS * NUM_INDIRECT_BLOCKS);
        uint32_t rem = triple_offset % (NUM_INDIRECT_BLOCKS * NUM_INDIRECT_BLOCKS);
        uint32_t double_block_offset = rem / NUM_INDIRECT_BLOCKS;
        uint32_t single_block_offset = rem % NUM_INDIRECT_BLOCKS;

        uint32_t *triple_indirect = read_block(i_node->triply_indirect_block_pointer);
        uint32_t *double_indirect = read_block(triple_indirect[triple_block_offset]);
        uint32_t *single_indirect = read_block(double_indirect[double_block_offset]);
        uint32_t block_ptr = single_indirect[single_block_offset];
        kfree_size(triple_indirect, PAGE_SIZE);
        kfree_size(double_indirect, PAGE_SIZE);
        kfree_size(single_indirect, PAGE_SIZE);
        return block_ptr;
    }
}

/**
 * Given an inode address, return the `struct inode` at that address.
 */
struct inode* find_inode(uint32_t addr) {
    struct inode *i_node;
    
    // Gather information on where the inode is
    uint32_t inode_block_group = ((addr - 1) / ext2_inodes_per_bgroup); // Which block group the inode is in
    uint32_t index = (addr - 1) % ext2_inodes_per_bgroup; // Index into the block group's inode_table

    // Retrieve the inode
    struct block_group_descriptor_table *bgdt = read_block(1);
    struct inode *inode_table = read_block(bgdt[inode_block_group].inode_block_start_block_addr);
    i_node = &inode_table[index];

    return i_node;
}

struct inode* get_inode(char* path) {
    // Starting from the root directory.
    struct inode *current_node = get_root_inode();
    char** save_ptr = &path;
    char *current_path_item = strtok_r(path, "/", save_ptr);
   
    // Read through each block sequentially.
    uint32_t block_number = 0;
    int32_t entries_to_search = current_node->count_hard_links;
    while (1) {
        // Determine where to read the entry from based on block pointer
        struct directory_entry *entry;
        if (block_number < 12) {
            // Direct block pointer
            entry = read_block(current_node->direct_block_pointer[block_number]);
        } else if (block_number < (ext2_block_size + 12)) {
            // Single indirect block pointer
            uint32_t *blocks = read_block(current_node->single_indirect_block_pointer); // Array of direct block pointers
            entry = read_block(blocks[block_number - 12]);
            kfree_size(blocks, PAGE_SIZE);
        } else { // Assume we're never using more than 16MiB for a single directory.
            PANIC("EXT2: Directory search exceeded 16MiB.");
        }
        
        printf("Entries to search for in dir: %d\n", entries_to_search);

        // Read through entire block of entries
        uint32_t block_idx = 0;
        printf("Searching for: \"%s\"\n", current_path_item);
        for (; block_idx < ext2_block_size;) {
            // Check that this entry is not empty.
            if (entry->inode == 0) {
                // Skip this entry.
                uint8_t *entry_addr = (uint8_t *) entry + entry->rec_len;
                entry = (struct directory_entry *) entry_addr;
                block_idx += entry->rec_len;
                continue;
            }

            // Check that this entry is valid.
            if (entry->rec_len < 8) {
                // Corrupted entry, retrieve next block.
                block_number++;
                break;
            }

            // Compare the name of the entry to the path.
            char entry_name[256];
            uint32_t n = entry->name_len_lsb_8;
            memcpy(entry_name, &entry->name, n);
            entry_name[n] = '\0';

            printf("Entry data:\n");
            printf("\tName: %s\n", entry_name);
            printf("\tInode: %d\n", entry->inode);
            printf("\tLength: %d\n", entry->name_len_lsb_8);
            printf("\tType: %d\n\n", entry->type_indicator);

            // Check name
            if (!strcmp(entry_name, current_path_item)) {
                struct inode *i_node = find_inode(entry->inode); // Retrieve the inode                
                current_path_item = strtok_r(NULL, "/", save_ptr); // Check the next path item

                if (current_path_item == NULL) {
                    // Last path item, return the i_node.
                    printf("End of path chain.\n");
                    return i_node;
                }

                // Not last item, traverse next dir level.
                kfree_size(current_node, PAGE_SIZE); // Free the old inode
                printf("Entering directory:\n\t%s\n", entry_name);
                current_node = i_node;

                // Reset things in prep of next dir traversal.
                block_number = 0 - 1; // Easier than handling logic for the break, just account for the increment later.
                block_idx = 0;
                entries_to_search = current_node->count_hard_links;
                break;
            }

            // Not the right entry, try the next.
            uint8_t *entry_addr = (uint8_t *) entry + entry->rec_len;
            entry = (struct directory_entry *) entry_addr;
            entries_to_search--;
            block_idx += entry->rec_len;

            // Break from search if all entries are checked
            if (entries_to_search < 0) {
                return NULL;
            }

        }

        // Entry not found, try next block.
        block_number++;
    }

    return NULL; // No inode found.
}

