#include <stdbool.h>

#include <bootloader/limine_requests.h>
#include <drivers/screen/colors.h>
#include <drivers/screen/screen.h>
#include <drivers/serial.h>
#include <kernel/init.h>
#include <kernel/memory/pmm.h>
#include <klib/math.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#include <arch/x86_64/drivers/lapic_timer.h>
#include <arch/x86_64/idt.h>
#endif

// TODO: Make the logging better
void kernel_init(void) {
    disable_interrupts();

    set_auto_flush(false);

    // 1. Serial
    serial_init();

    // 2. Limine
    limine_init();

    // 3. Physical Memory Manager
    pmm_init();

    // 4. Screen
    screen_init();
    kprintf_default("Initializing screen driver... ");
    kprintf_default("[OK]\n");

    // 5. Interrupt Descriptor Table
    kprintf_default("Initializing IDT... ");
    idt_init();
    kprintf_default("[OK]\n");

#ifdef CONFIG_ARCH_X86_64
    // 6. LAPIC Timer
    kprintf_default("Initializing LAPIC timer... ");
    lapic_timer_device_init(LAPIC_TIMER_DIV_16);
    idt_register_handler(LAPIC_TIMER_VECTOR, lapic_timer_irq_handler);
    kprintf_default("[OK]\n");
#endif

    enable_interrupts();

    screen_flush();

    set_auto_flush(true);
}