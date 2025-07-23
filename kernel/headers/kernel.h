#ifndef KERNEL_H
#define KERNEL_H

#include "../../klib/headers/stdio.h"

// Syscall numbers are defined once here, and once again in the userspace.

#define SYS_PUTC            1
#define SYS_GETC            2
#define SYS_EXIT            3
#define SYS_OPEN_FILE       4   
#define SYS_CLOSE_FILE      5
#define SYS_EXEC            6

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

#endif
