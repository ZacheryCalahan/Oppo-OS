#include "syscall.h"
#include "kernel.h"
#include "fs/fs.h"
#include "schedule/proc.h"
#include "../arch/paging.h"

void syscall_entry(struct trap_frame *f) {
    switch (f->a3) {
        case SYS_PUTC: { // To be Deprecated
            putc(f->a0);
            break;
        }
        case SYS_GETC: { // To be Deprecated
            while (1) {
                long c = getc();
                if (c >= 0) {
                    f->a0 = c;
                    break;
                }

                yield(); // I do not recall why this is here?
            }
            break;
        }
        case SYS_EXIT: {
            proc_exit();
            PANIC("Unreachable!");
        }
        case SYS_READ: {
            void *arg_a1 = vaddr_to_paddr(current_proc->page_table, f->a1); // `void *buf` (paddr)
            int64_t ret = read_file(f->a0, arg_a1, f->a2);
            f->a0 = ret;
            break;
        }
        case SYS_OPEN: {
            char *arg_a0 = (char *) vaddr_to_paddr(current_proc->page_table, f->a0); // `char *path` (paddr)
            int32_t ret = open_file(arg_a0, f->a1);
            f->a0 = ret;
            break;
        }
        case SYS_CLOSE: {
            close_file(f->a0);
            break;
        }
        
        default:
            PANIC("Unexpected syscall: 0x%x\n", f->a3);
    }
}