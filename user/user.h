#ifndef USER_H
#define USER_H

#include <stdint.h>

__attribute__((noreturn)) void exit(void);

int32_t open(const char* path, int access);
int64_t read(int32_t fd, void *buf, uint32_t size);
void close(int32_t fd);

void putc(const char c);
int getc(void);
int syscall(int sysno, int arg0, int arg1, int arg2);

#endif