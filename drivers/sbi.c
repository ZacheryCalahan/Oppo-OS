#include "headers/sbi.h"

struct sbiret sbi_call(uint64_t arg0,
    uint64_t arg1,
    uint64_t arg2,
    uint64_t arg3,
    uint64_t arg4,
    uint64_t arg5,
    uint64_t fid,
    uint64_t eid) {

    register uint64_t a0 __asm__("a0") = arg0;
    register uint64_t a1 __asm__("a1") = arg1;
    register uint64_t a2 __asm__("a2") = arg2;
    register uint64_t a3 __asm__("a3") = arg3;
    register uint64_t a4 __asm__("a4") = arg4;
    register uint64_t a5 __asm__("a5") = arg5;
    register uint64_t a6 __asm__("a6") = fid;
    register uint64_t a7 __asm__("a7") = eid;

    __asm__ __volatile__ (
        "ecall"
        : "=r"(a0), "=r"(a1)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
            "r"(a6), "r"(a7)
        : "memory"
    );

    return (struct sbiret) {.error = a0, .value = a1};
        
}