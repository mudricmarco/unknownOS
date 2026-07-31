#include <drivers/screen/colors.h>
#include <klib/math.h>

#include <drivers/serial.h>
#include <bootloader/limine_requests.h>
#include <drivers/screen/screen.h>
#include <kernel/memory/pmm.h>
#include <kernel/init.h>
#include <stdbool.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/idt.h>
#include <arch/x86_64/cpu.h>
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

    enable_interrupts();

    screen_flush();

    set_auto_flush(true);
}