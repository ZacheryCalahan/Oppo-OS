#include "headers/ext2.h"
#include "headers/blkio.h"
#include "../kernel/headers/kernel.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"

uint32_t block_size;
uint32_t inode_count;
uint32_t block_count;
uint32_t blocks_per_bgroup;
uint32_t inodes_per_bgroup;
uint32_t inode_size_bytes;
int is_read_only;

uint32_t inode_addr_to_block_group(uint32_t inode_addr);

void init_ext2() {
    struct s_block *superblock = (struct s_block *) kalloc(1);
    read_write_disk(superblock, 2, 0); // Read in the super block (1024 bytes)
    read_write_disk(superblock + SECTOR_SIZE, 3, 0);
    if (superblock->signature != EXT2_SIGNATURE) {
        PANIC("EXT2: Superblock not found.");
    }

    // Extracts
    block_size = (1024 << superblock->block_size_shift);
    inode_count = superblock->inode_count;
    block_count = superblock->block_count;
    blocks_per_bgroup = superblock->blocks_in_block_group;
    inodes_per_bgroup = superblock->inodes_in_block_group;
    inode_size_bytes = superblock->esb.size_inode_bytes;

    // Check required features (if major >= 1.)
    if (superblock->major_version >= 1) {
        printf("EXT2: version %d.%d\n", superblock->major_version, superblock->minor_version);
        
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
        if (req_w_feats & RO_FEAT_SPARSE_SBLOCK_GROUP_DES_T) {
            printf("EXT2: Feature `Sparse superblocks and group descriptor tables` found but not supported.\n");
            is_read_only = 1;
        } 
        if (req_w_feats & RO_FEAT_64BIT_FILE_SIZE) {
            printf("EXT2: Feature `64-bit file sizes` found but not supported.\n");
            is_read_only = 1;
        }
        if (req_w_feats & RO_FEAT_DIR_CONTENTS_BINARY_TREE) {
            printf("EXT2: Feature `Directory contents stored in binary tree` found but not supported.\n");
            is_read_only = 1;
        }
        if (is_read_only) {
            printf("EXT2: File system mounted as read-only!\n");
        }
        
        // Optional features (we don't use them, don't even check them.)



        
    }
    

    
    
}



uint32_t inode_addr_to_block_group(uint32_t inode_addr) {
    return (inode_addr - 1) / inodes_per_bgroup;
}


