#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <klib/mem.h>
#include <klib/string.h>
#include "arch/x86_64/cpu.h"

void enable_interrupts(void) {
    asm volatile ("sti");
}

void disable_interrupts(void) {
    asm volatile ("cli");
}

// Halt and Catch Fire - stop the CPU in a low-power halted loop
void hcf(void) {
    for (;;) {
        asm volatile ("hlt");
    }
}

void halt(void) {
    asm volatile ("hlt");
}

void halt_sti(void) {
    asm volatile ("sti; hlt");
}

void flush_tlb(uint64_t virt_addr) {
    asm volatile ("invlpg (%0)" :: "r" (virt_addr) : "memory");
}