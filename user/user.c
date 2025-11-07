#include "user.h"

enum SYSTEM_CALL_ID {
    /* Files */
    SYS_OPEN,   // open()
    SYS_CLOSE,  // close()
    SYS_READ,   // read()
    SYS_WRITE,  // write()
    SYS_IOCTL,  // ioctl()
    SYS_STAT,   // stat()

    /* Processes */
    SYS_EXEC,   // exec()
    SYS_EXIT,   // exit()
    SYS_BRK,    // brk()

    /* Temporary */
    SYS_PUTC,   // Only used until devices are mapped to the file system.
    SYS_GETC,   // 


};

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

int32_t open(const char* path, int access) {
    return syscall(SYS_OPEN, (int) path, access, 0);
}

int64_t read(int32_t fd, void *buf, uint32_t size) {
    return syscall(SYS_READ, fd, (int) buf, size);
}

void close(int32_t fd) {
    syscall(SYS_CLOSE, fd, 0, 0);
}

void putc(const char c) {
    syscall(SYS_PUTC, c, 0, 0);
}

int getc(void) {
    return syscall(SYS_GETC, 0, 0, 0);
}
