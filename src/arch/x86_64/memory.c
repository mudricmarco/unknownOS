#include <arch/x86_64/memory.h>
#include <stdint.h>

uint64_t get_current_cr3(void) {
    uint64_t cr3_value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_value));
    return cr3_value;
}

void write_cr3(uint64_t phys_addr) {
    asm volatile("mov %0, %%cr3" :: "r"(phys_addr) : "memory");
}