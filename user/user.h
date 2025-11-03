#ifndef USER_H
#define USER_H

__attribute__((noreturn)) void exit(void);
void putc(const char c);
int getc(void);
int syscall(int sysno, int arg0, int arg1, int arg2);

#endif