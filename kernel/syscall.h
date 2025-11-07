#ifndef SYSCALL_H
#define SYSCALL_H

#include "../arch/trap.h"
#include <stdint.h>

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


/**
 * 
 */
void syscall_entry(struct trap_frame *f);

#endif