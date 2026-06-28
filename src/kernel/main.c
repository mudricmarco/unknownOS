#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>
#include <kernel/panic.h>
#include <arch/cpu.h>
#include <klib/string.h>
#include <klib/math.h>
#include <kernel/init.h>
#include <kernel/memory/pmm.h>

#define kversion "0.1.0"

#define DIRECT_VRAM_WRITE false

// --- KERNEL ENTRY POINT ---
void kmain(void) {

    // Initialize the kernel subsystems
    kernel_init();

    // ! That's a very bad hack, but it works for now. We should implement a proper delay mechanism in the future.
    delay(900000000); // Delay to allow users to read the boot messages before clearing the screen

    set_auto_flush(false);

    screen_clear(COLOR_BLACK, DIRECT_VRAM_WRITE);

    kprintf_default_scale(COLOR_AQUA, DIRECT_VRAM_WRITE,
            "   __  __      __                             ____  _____\n"
                "  / / / /___  / /______  ____ _      ______  / __ \\/ ___/\n"
                " / / / / __ \\/ //_/ __ \\/ __ \\ | /| / / __ \\/ / / /\\__ \\\n"
                "/ /_/ / / / / ,< / / / / /_/ / |/ |/ / / / / /_/ /___/ /\n"
                "\\____/_/ /_/_/|_/_/ /_/\\____/|__/|__/_/ /_/\\____//____/\n"
                "\n");

    kprintf_default_scale(COLOR_WHITE, DIRECT_VRAM_WRITE, 
            " Kernel Version: %C%s%C\n"
            " System is ready.\n",
            COLOR_YELLOW_ORANGE, kversion, COLOR_WHITE);

    screen_flush();

    set_auto_flush(true);
            
    hcf();
}