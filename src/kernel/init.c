#include <stdbool.h>

#include <bootloader/limine_requests.h>
#include <drivers/screen/colors.h>
#include <drivers/screen/screen.h>
#include <drivers/serial.h>
#include <kernel/init.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/panic.h>
#include <klib/math.h>
#include <drivers/keyboard.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#include <arch/x86_64/drivers/lapic_timer.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/drivers/ioapic.h>
#include <arch/x86_64/memory.h>
#endif

static void log_step(const char *msg) {
    kprintf_default("[INIT] %s... ", msg);
}

static void log_ok(void) {
    kprintf_default("[  OK  ]\n");
}

void kernel_init(void) {
    disable_interrupts();
    set_auto_flush(false);

    // 1. Early serial communication
    serial_init();
    limine_init();

    // 2. Physical Memory Manager (required by screen_init for backbuffer allocation)
    pmm_init();

    // 3. Screen Driver
    screen_init();

    log_step("Initializing PMM");
    log_ok();

    log_step("Initializing Screen Driver");
    log_ok();

    // 4. Virtual Memory Manager + CR3 switch verification
    log_step("Initializing VMM");
    uint64_t old_cr3 = get_current_cr3();

    if (!vmm_init()) {
        kernel_panic("Failed to initialize VMM!");
    }

    uint64_t new_cr3 = get_current_cr3();
    log_ok();
    kprintf_default("          ---> CR3 switched: 0x%x -> 0x%x\n", old_cr3, new_cr3);

    // 5. Interrupt Descriptor Table
    log_step("Initializing IDT");
    idt_init();
    log_ok();

#ifdef CONFIG_ARCH_X86_64
    // 6. LAPIC Timer
    log_step("Initializing LAPIC timer");
    lapic_timer_device_init(LAPIC_TIMER_DIV_16);
    idt_register_handler(LAPIC_TIMER_VECTOR, lapic_timer_irq_handler);
    log_ok();
#endif

    // 7. Keyboard Driver
    log_step("Initializing keyboard driver");
    keyboard_init();
    idt_register_handler(IOAPIC_IDT_VECTOR, keyboard_irq_handler);
    log_ok();

    enable_interrupts();

    screen_flush();
    set_auto_flush(true);
}