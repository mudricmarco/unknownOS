#include <drivers/screen/colors.h>
#include <klib/math.h>

#include <bootloader/limine_requests.h>
#include <drivers/screen/screen.h>
#include <kernel/memory/pmm.h>
#include <kernel/init.h>
#include <stdbool.h>

void kernel_init(void) {

    set_auto_flush(false);

    // 1. Limine
    limine_init();

    init_physical_memory();

    // 2. Screen
    screen_init();
    kprintf_default("Initializing screen driver... ");
    kprintf_default("[OK]\n");

    screen_flush();

    set_auto_flush(true);
}