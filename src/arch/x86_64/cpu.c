#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <klib/mem.h>
#include <klib/string.h>
#include "arch/cpu.h"

// Halt and Catch Fire - stop the CPU in a low-power halted loop
void hcf(void) {
    for (;;) {
        asm volatile ("hlt");
    }
}

// Debug Function to create a delay loop (not precise and slow, but sufficient for simple timing)
void delay(uint64_t count) {
    volatile uint64_t i;
    for (i = 0; i < count; i++) {
        asm volatile("nop");
    }
}