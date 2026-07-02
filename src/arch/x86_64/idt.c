#include <arch/x86_64/idt.h>
#include <arch/x86_64/cpu.h>
#include <kernel/panic.h>
#include <stdint.h>
#include <stddef.h>

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

void kernel_panic_detailed(struct registers* regs) {
    disable_interrupts();

    char buffer[2048];

    create_string_buf(
        buffer, 
        sizeof(buffer),
        "unknownOS has encountered a fatal error and must be shut down.\n"
        "Detail: %s (Vector: %d, Error Code: 0x%x)\n\n"
        "--- STATE DUMP (REGISTERS) ---\n"
        "RIP: 0x%x    CS:  0x%x    RFLAGS: 0x%x\n"
        "RSP: 0x%x    SS:  0x%x\n\n"
        "RAX: 0x%x    RBX: 0x%x    RCX:    0x%x    RDX: 0x%x\n"
        "RSI: 0x%x    RDI: 0x%x    RBP:    0x%x\n"
        "R8:  0x%x    R9:  0x%x    R10:    0x%x    R11: 0x%x\n"
        "R12: 0x%x    R13: 0x%x    R14:    0x%x    R15: 0x%x\n\n"
        "--------------------------------------------------------------------------------\n"
        "The system has been halted. Please reboot your hardware manually.\n",
        exception_names[regs->int_no], regs->int_no, regs->error_code,
        regs->rip, regs->cs, regs->rflags, regs->rsp, regs->ss,
        regs->rax, regs->rbx, regs->rcx, regs->rdx,
        regs->rsi, regs->rdi, regs->rbp,
        regs->r8,  regs->r9,  regs->r10, regs->r11,
        regs->r12, regs->r13, regs->r14, regs->r15
    );

    kernel_panic(buffer);
}

struct idt_entry idt[256];
struct idt_ptr idt_pointer;

static isr_handler_t interrupt_handlers[256] = {NULL};

extern uint64_t isr_stub_table[];

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
            kernel_panic_detailed(regs);
        }
    }
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
}