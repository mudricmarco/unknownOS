#pragma once

#include <stdint.h>
#include <stdbool.h>

static inline void enable_interrupts(void) {
    asm volatile ("sti");
}

static inline void disable_interrupts(void) {
    asm volatile ("cli");
}

void hcf(void);

void delay(uint64_t count);