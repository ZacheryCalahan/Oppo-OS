#include "headers/fs.h"
#include "../drivers/headers/fat.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"

/*
    API to work with filesystems, non-specific fs implementations should be here.
*/

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

void close_file(struct FILE *file) {
    kfree_order(file, order_for_pages(get_pages_from_bytes((uint64_t) file->size)));
}

