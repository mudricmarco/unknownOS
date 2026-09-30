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
#include <drivers/keyboard.h>
#include <kernel/memory/heap.h>
#include <kernel/thread/thread.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#endif

#define kversion "0.2.0"

#define DIRECT_VRAM_WRITE false

void print_hello_thread(void *arg) {
    const char *message = (const char *)arg;
    for (int i = 0; i < 10; i++) {
        kprintf_default_scale(COLOR_WHITE, DIRECT_VRAM_WRITE, "%s", message);
        sleep_ms(1000);
    }
}

// --- KERNEL ENTRY POINT ---
void kmain(void) {
    // Initialize the kernel subsystems
    kernel_init();

    screen_flush();

    sleep_ms(2000);

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
        
    thread_t *thread1 = thread_create("Thread1", print_hello_thread, "Hello from Thread 1!\n");

    // Test if all the thread info is correct
    kprintf_default_scale(COLOR_WHITE, DIRECT_VRAM_WRITE,
            " Thread ID: %C%d%C\n"
            " Thread Name: %C%s%C\n"
            " Thread State: %C%d%C\n"
            " Thread Kernel Stack Base: %C%x%C\n"
            " Thread Kernel Stack Size: %C%d%C\n",
            COLOR_YELLOW_ORANGE, thread1->id, COLOR_WHITE,
            COLOR_YELLOW_ORANGE, thread1->name, COLOR_WHITE,
            COLOR_YELLOW_ORANGE, thread1->state, COLOR_WHITE,
            COLOR_YELLOW_ORANGE, thread1->kernel_stack_base, COLOR_WHITE,
            COLOR_YELLOW_ORANGE, thread1->kernel_stack_size, COLOR_WHITE);

    screen_flush();

    set_auto_flush(true);

    while (1) {
        char c = keyboard_getchar();

        if (c != 0) {
            kprintf_default_scale(COLOR_WHITE, DIRECT_VRAM_WRITE, "%C%c%C", COLOR_YELLOW_ORANGE, c, COLOR_WHITE);
        }
    }

    hcf();
}