/*
    OpenSBI related code goes here!
*/

#ifndef SBI_H
#define SBI_H

#include <stdint.h>

struct sbiret {
    uint64_t error;
    uint64_t value;
};

struct sbiret sbi_call(uint64_t arg0,
                       uint64_t arg1,
                       uint64_t arg2,
                       uint64_t arg3,
                       uint64_t arg4,
                       uint64_t arg5,
                       uint64_t fid,
                       uint64_t eid);


struct sbiret sbi_putc(const char c);
long sbi_getc(void); 
#endif SBI_H