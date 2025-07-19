#ifndef FS_H
#define FS_H

#include <stddef.h>

#define FILES_MAX 2

struct file {
    int in_use;
    char name[100];
    char data[4096];
    size_t size;
};

#endif