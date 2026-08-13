#include "cpu.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>
#include <kernel/panic.h>
#include <klib/string.h>
#include <klib/math.h>
#include <kernel/init.h>
#include <kernel/timer/timer.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#endif

#define kversion "0.1.0"

#define DIRECT_VRAM_WRITE false

// --- KERNEL ENTRY POINT ---
void kmain(void) {
    // Initialize the kernel subsystems
    kernel_init();

    screen_flush();

    sleep_ms(5000);

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