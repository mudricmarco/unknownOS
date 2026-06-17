#include <drivers/screen/colors.h>
#include <klib/math.h>

#include <bootloader/limine_requests.h>
#include <drivers/screen/screen.h>
#include <kernel/memory/pmm.h>
#include <kernel/init.h>

void kernel_init(void) {
    // 1. Limine
    limine_init();

    // 2. Screen
    screen_init();
    kprintf_default("Initializing screen driver... ");
    kprintf_default("[OK]\n");

    // 3. PMM
    kprintf_default("Initializing memory manager.. ");
    init_physical_memory();
    kprintf_default("[OK]\n");
    kprintf_default("   -> Physical memory available: %d MB\n", byte_to_mebibyte(get_total_memory_size()));
}