#pragma once

#include <stdint.h>

// Structure representing the CPU registers saved during an interrupt or exception
struct registers {
    // Manually pushed registers
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;

    // Pushed by the assembly stub
    uint64_t int_no, error_code;

    // Pushed by the processor automatically
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));