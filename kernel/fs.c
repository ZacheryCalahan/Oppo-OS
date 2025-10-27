#include "headers/fs.h"
#include "../drivers/headers/fat.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"

/*
    API to work with filesystems, non-specific fs implementations should be here.
*/
#ifndef FAT_H
    PANIC("File system not recognized, halting!");
#endif

struct FILE *open_file(const char* path) {
    // Find the file by the path
    uint32_t file_size = 0;
    void* file_data = fat32_get_file_by_path(path, &file_size);
    
    if (file_data == NULL) {
        printf("File \"%s\" not found.\n", path);
        return NULL;
    }

    struct FILE *new_file = (struct FILE *) kalloc(get_pages_from_bytes(file_size));
    memcpy(new_file->data, file_data, file_size); // Insert data
    new_file->in_use = 1;
    new_file->size = file_size;

    return new_file;
}

void flush_file(const char* path, struct FILE *file) {
    // This is VERY primitive and assumes that the caller has updated the file data (contigious memory!),
    // the file size, and did not mess with the type of file (can't change file name!). 
    fat32_write_file_by_path(path, file->data, file->size, 0);
}

void close_file(struct FILE *file) {
    file->in_use = 0;
    file->size = 0;
    kfree_order(file, order_for_pages(get_pages_from_bytes((uint64_t) file->size)));
}

