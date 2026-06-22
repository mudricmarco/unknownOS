#include <stdint.h>

uint64_t get_current_cr3(void) {
    uint64_t cr3_value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_value));
    return cr3_value;
}