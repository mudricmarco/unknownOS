#pragma once

#include <stdint.h>
#include <arch/x86_64/registers.h>

typedef void (*isr_handler_t) (struct registers* regs);

// Structure representing an entry in the Interrupt Descriptor Table (IDT)
struct idt_entry {
    uint16_t isr_low;    // Offset bits 0 to 15
    uint16_t kernel_cs;  // Code segment selector
    uint8_t ist;         // Interrupt Stack Table
    uint8_t  attributes; // Attributes
    uint16_t isr_mid;    // Offset bits 16 to 31
    uint32_t isr_high;   // Offset bits 32 to 63
    uint32_t zero;       // Reserved
}__attribute__((packed));

// Structure representing the pointer to the IDT
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));


extern struct idt_entry idt[256];
extern struct idt_ptr idt_pointer;

void idt_init(void);
void idt_register_handler(uint8_t vector, isr_handler_t handler);
