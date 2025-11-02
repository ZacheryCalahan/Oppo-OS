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
struct FILE_TABLE {
    struct FILE file_descriptors[256];
};

int open_file(const char* path) {
    // Retrieve the inode for the file from the path
    struct inode file_inode;

}

void* read_file(struct FILE* file) {

}

void flush_file(struct FILE *file) {

}

void close_file(struct FILE *file) {
    
}

