#include "headers/fs.h"
#include "../drivers/headers/fat.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"

/*
    API to work with filesystems, non-specific fs implementations should be here.
    Considered the VFS.
*/

/**
 * Retains all opened files in use.
 */
struct file_descriptor_table {
    struct FILE file_descriptors[256];
} file_desc_table;

int open_file(const char* path) {
    // Retrieve the inode for the file from the path
    struct inode file_inode;

}

void* read_file(filedec_t file) {

}

void flush_file(filedec_t file) {

}

void close_file(filedec_t file) {
    
}

