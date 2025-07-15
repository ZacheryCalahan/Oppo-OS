#ifndef RISCV_H
#define RISCV_H

static inline int is_aligned(uint64_t value, uint64_t align) {
    if (align == 0)
        align = sizeof(void*);
    return value % align == 0;
}


#endif