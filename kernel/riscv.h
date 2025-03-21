#ifndef RISCV_H
#define RISCV_H

#include <stdint.h>

#define reg_t uint64_t // We're using a 64bit processor, this is the size of the registers.

struct context {
    reg_t ra;
    reg_t sp;

    // callee saved registers
    reg_t s0;
    reg_t s1;
    reg_t s2;
    reg_t s3;
    reg_t s4;
    reg_t s5;
    reg_t s6;
    reg_t s7;
    reg_t s8;
    reg_t s9;
    reg_t s10;
    reg_t s11;
};

#define CLINT 0x20000000

// Retrieve the id of the hart
static inline reg_t r_mhartid() {
    reg_t x;
    asm volatile("csrr %0, mhartid" : "=r" (x) );
    return x;
}

// Read from `mstatus`
static inline reg_t r_mstatus() {
    reg_t x;
    asm volatile("csrr %0, mstatus" : "=r" (x) );
    return x;
}

// Write to `mtvec`
static inline void w_mtvec(reg_t x) {
    asm volatile("csrw mtvec, %0" : : "r" (x));
}

// M-Mode Interrupt Enable
#define MIE_MEIE (1 << 11) // External
#define MIE_MTIE (1 << 7)  // Timer
#define MIE_MSIE (1 << 3)  // Software

// Read from `mie`
static inline reg_t r_mie() {
    reg_t x;
    asm volatile("csrr %0, mie" : "=r" (x) );
    return x;
}

// Write to `mie`
static inline void w_mie(reg_t x) {
    asm volatile("csrw mie, %0" : : "r" (x));
}

/* Trap Handler Registers */

static inline reg_t r_mcause() {
    reg_t x;
    asm volatile("csrr %0, mcause" : "=r" (x) );
    return x;
}

static inline reg_t r_mepc() {
    reg_t x;
    asm volatile("csrr %0, mepc" : "=r" (x));
    return x;
}

static inline void w_mepc(reg_t x) {
    asm volatile("csrw mepc, %0" : : "r" (x));
}

static inline reg_t r_a1() {
    reg_t x;
    asm volatile("mv %0, a1" : "=r" (x));
    return x;
}

#endif