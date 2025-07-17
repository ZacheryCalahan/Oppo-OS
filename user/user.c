#include "headers/user.h"

#define SYS_PUTC 1
#define SYS_GETC 2
#define SYS_EXIT 3

extern char __stack_top[];

__attribute__((noreturn)) void exit(void) {
    syscall(SYS_EXIT, 0, 0, 0);
    for (;;); // just in case?
}

__attribute__((section(".text.start")))
__attribute__((naked))
void start(void) {
    __asm__ __volatile__ (
        "mv sp, %[stack_top] \n"
        "call main \n"
        "call exit \n"
        :: [stack_top] "r" (__stack_top)
    );
}

int syscall(int sysno, int arg0, int arg1, int arg2) {
    register int a0 __asm__("a0") = arg0;
    register int a1 __asm__("a1") = arg1;
    register int a2 __asm__("a2") = arg2;
    register int a3 __asm__("a3") = sysno;

    __asm__ __volatile__(
        "ecall"
        : "=r" (a0)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3)
        : "memory"
    );

    return a0;
}

void putc(const char c) {
    syscall(SYS_PUTC, c, 0, 0);
}

int getc(void) {
    return syscall(SYS_GETC, 0, 0, 0);
}
