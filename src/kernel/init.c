#include "idt.h"
#include <drivers/screen/colors.h>
#include <klib/math.h>

#include <bootloader/limine_requests.h>
#include <drivers/screen/screen.h>
#include <kernel/memory/pmm.h>
#include <kernel/init.h>
#include <stdbool.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/idt.h>
#endif

void kernel_init(void) {

    set_auto_flush(false);

    // 1. Limine
    limine_init();

    // 2. Physical Memory Manager
    init_pmm();

    // 3. Screen
    screen_init();
    kprintf_default("Initializing screen driver... ");
    kprintf_default("[OK]\n");

    kprintf_default("Initializing IDT... ");
    idt_init();
    kprintf_default("[OK]\n");

    screen_flush();

    set_auto_flush(true);
}