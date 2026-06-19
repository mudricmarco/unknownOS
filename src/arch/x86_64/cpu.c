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

//! A shitty delay function that just loops for a given count. 
//! This is not a precise timing mechanism and should be replaced with a proper timer-based delay in the future.
void delay(uint64_t count) {
    volatile uint64_t i;
    for (i = 0; i < count; i++) {
        asm volatile("nop");
    }
}