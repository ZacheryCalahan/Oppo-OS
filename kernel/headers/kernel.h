#ifndef KERNEL_H
#define KERNER_H

#include "../../klib/headers/stdio.h"

#define SYS_PUTC 1
#define SYS_GETC 2
#define SYS_EXIT 3

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

#endif