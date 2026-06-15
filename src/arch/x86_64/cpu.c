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

//! Detect if we're running inside QEMU by checking the CPUID signature, for debugging purpose
bool is_running_on_qemu(void) {
    uint32_t eax, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x40000000));
    
    char signature[13];
    memcpy(signature, &ebx, 4);
    memcpy(signature + 4, &ecx, 4);
    memcpy(signature + 8, &edx, 4);
    signature[12] = '\0';

    return (strcmp(signature, "TCGTCGTCGTCG") == 0 || strcmp(signature, "KVMKVMKVM") == 0);
}

//! Exit QEMU
void qemu_exit(void) {
    asm volatile ("outw %0, %1" : : "a"((uint16_t)0x2000), "Nd"((uint16_t)0x604));
}