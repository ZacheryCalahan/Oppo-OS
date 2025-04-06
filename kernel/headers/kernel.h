#ifndef KERNEL_H
#define KERNER_H

#include "memlayout.h"

// All available log types for kernel logs
enum LogType {
    ERROR,
    WARN,
    INFO,
    PANIC,
    OK,
    FAILED
};

// Get the intended string of a log type by its enum.
static const char *log_type_strings[] = {
    [ERROR]   = "\033[0;31m ERROR  ",
    [WARN]    = "\033[0;33m WARN   ",
    [INFO]    = " INFO   ",
    [PANIC]   = "\033[0;35m PANIC  ",
    [OK]      = "\033[0;32m OK     ",
    [FAILED]  = "\033[0;31m FAILED "
};

// Dedicated log function for writing kernel initialization logs.
void klog(char* log, enum LogType type);

void panic(void);
#endif