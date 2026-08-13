#include <arch/x86_64/idt.h>
#include <arch/x86_64/cpu.h>
#include <arch/x86_64/io.h>
#include <arch/x86_64/lapic.h>
#include <kernel/panic.h>
#include <stdint.h>
#include <stddef.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

static const char* exception_names[] = {
    "Division By Zero", "Debug", "Non-Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "SIMD Floating-Point",
    "Virtualization Exception", "Control Protection Exception", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Security Exception", "Reserved"
};

struct idt_entry idt[256];
struct idt_ptr idt_pointer;

static isr_handler_t interrupt_handlers[256] = {NULL};

extern uint64_t isr_stub_table[]; // Array of ISR stub addresses defined in assembly

static void idt_set_descriptor(uint8_t vector, uint64_t isr_address, uint8_t attributes, uint16_t _kernel_cs) {

    idt[vector].isr_low    = (uint16_t)(isr_address & 0xFFFF);
    idt[vector].isr_mid    = (uint16_t)((isr_address >> 16) & 0xFFFF);
    idt[vector].isr_high   = (uint32_t)((isr_address >> 32) & 0xFFFFFFFF);
    
    idt[vector].attributes = attributes;
    idt[vector].kernel_cs  = _kernel_cs;
    idt[vector].ist        = 0; // For now, i'm not using the IST feature, so it's set to 0. Mabybe in the future.
    idt[vector].zero       = 0;
}

void idt_register_handler(uint8_t vector, isr_handler_t handler) {
    interrupt_handlers[vector] = handler;
}

void idt_handler_c(struct registers* regs) {
    if (interrupt_handlers[regs->int_no] != NULL) {
        isr_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        if (regs->int_no < 32) {
            kernel_panic_detailed(regs, exception_names[regs->int_no]);
        }
    }
}

// Disable the legacy PIC to prevent conflicts with the APIC
static void pic_disable(void) {
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    outb(PIC1_DATA, 0xFF);
    io_wait();
    outb(PIC2_DATA, 0xFF);
    io_wait();
}

void idt_init(void) {
    uint16_t current_cs;
    __asm__ volatile("mov %%cs, %0" : "=r"(current_cs));

    for (int i = 0; i < 256; i++) {
        idt_set_descriptor(i, isr_stub_table[i], 0x8E, current_cs);
    }

    idt_pointer.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_pointer.base  = (uint64_t)&idt;

    if (idt_pointer.base == 0) {
        kernel_panic("IDT base address is NULL.");
    }

    __asm__ volatile ("lidt %0" : : "m"(idt_pointer));

    pic_disable();

    lapic_init();
}