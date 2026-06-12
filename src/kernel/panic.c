#include <kernel/panic.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>

static void hcf(void) {
    for (;;) {
        asm volatile ("hlt");
    }
}

void kernel_panic(const char *message) {
    screen_clear(COLOR_BLACK, true);
    kprint_default_scale("\n\n!!! KERNEL PANIC !!!\n", COLOR_RED, true);
    kprint_default_scale("\n", COLOR_WHITE, true);
    kprint_default_scale(message, COLOR_WHITE, true);
    kprint_default_scale("\n\nSystem halted.", COLOR_RED, true);

    hcf();
}