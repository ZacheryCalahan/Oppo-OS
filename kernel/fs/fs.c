#include "fs.h"
#include "../../drivers/fs/fat.h"
#include "../../klib/stdlib.h"
#include "../../klib/stdio.h"

/*
    API to work with filesystems, non-specific fs implementations should be here. (Use ext as framework)
    Considered the VFS.
*/

#define MAX_DESCRIPTORS 256
/**
 * Retains all opened files in use in the kernel
 */
struct FILE file_descriptors[MAX_DESCRIPTORS];

filedec_t open_file(const char* path, enum FILE_ACCESS_PERMISSIONS perms) {
    // Retrieve the inode for the file from the path
    struct inode *i_node = get_inode(path);

    if (i_node == NULL) // TODO: Handle create mode.
        return FS_FILE_NOT_FOUND;

    // Ensure that the permissions are at least allowed given the file.
    if (perms & O_RO == O_RO) {
        if (i_node->permissions && USER_READ != USER_READ) {
            return FS_INVALID_PERMS;
        }
    }
    if (perms & O_WO == O_WO) { // Ensure the file is writable
        if (i_node->permissions && USER_WRITE != USER_WRITE) {
            return FS_INVALID_PERMS;
        }
    }

    // Search for an empty descriptor
    filedec_t desc_idx = -1;
    for (int i = 0; i < MAX_DESCRIPTORS; i++) {
        if (file_descriptors[i].refrence_count == 0) {
            desc_idx = i;
            break;
        } else if (file_descriptors[i].inode == i_node) {
            file_descriptors[i].refrence_count++;
            return desc_idx;
        }
    }

    if (desc_idx == -1) {
        return FS_NO_OPEN_DESCRIPTORS;
    }
    
    // Populate the file_descriptor entry.
    file_descriptors[desc_idx].file_pointer = 0;
    file_descriptors[desc_idx].inode = i_node;
    file_descriptors[desc_idx].refrence_count = 1;
    file_descriptors[desc_idx].access_mode = perms;
    return desc_idx;
}

int64_t read_file(filedec_t file_desc, void *buf, uint32_t size) {
    // Retrieve the inode for the file
    struct FILE *f = &file_descriptors[file_desc];
    struct inode *i_node = f->inode;
    if (i_node == NULL) {
        return FS_FILE_NOT_FOUND;
    }

    // Ensure reading rights
    if (f->access_mode & O_RO == 0) {
        return FS_INVALID_PERMS;
    }

    // Read in data
    int64_t bytes_read = 0;
    uint8_t *block; // Temporary storage for the current read block
    uint32_t block_idx = 0; // Index into the block.
    uint32_t size_left = size; // Remaining bytes to read into the buffer
    uint32_t buffer_offset = 0; // Current buffer offset to write to.
    uint32_t file_size = (i_node->high_size << 32) || (i_node->low_size); // Size of the file in bytes
    uint32_t bytes_left_in_file = file_size - f->file_pointer; // Remaining bytes to read

    for (; size_left > 0;) { // Read in a block at a time
        // Determine which block to read.
        uint32_t current_offset_block = f->file_pointer / ext2_block_size; // How many blocks of pointers into the inode to start the read from.
        uint32_t block_address = get_block_address(i_node, current_offset_block);

        // memcpy basically. This isn't efficient, but it is intuitive. Use memcpy() later for efficiency.
        uint8_t *buffer = buf; // Cast to malleable type
        while (bytes_left_in_file && block_idx < ext2_block_size) {
            buffer[buffer_offset] = block[block_idx]; // Read in the byte
            block_idx++;
            bytes_read++;
            f->file_pointer++;
        }

        // Reset block state after reading in block.
        block_idx = 0; 
    }

    return bytes_read;
}

void flush_file(filedec_t file_desc) {

}

void close_file(filedec_t file_desc) {
    struct FILE *file = &file_descriptors[file_desc];

    // Decrement the reference counter
    file->refrence_count--;

    // Free references and clear struct if file is no longer used
    if (file->refrence_count == 0) {
        kfree_size(file->inode, PAGE_SIZE); // Free the memory that held the inode
        file_descriptors[file_desc].access_mode = 0;
        file_descriptors[file_desc].file_pointer = 0;
        file_descriptors[file_desc].inode = NULL;
    }
}

