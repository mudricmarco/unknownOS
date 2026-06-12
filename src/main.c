#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <bootloader/limine.h>
#include <bootloader/limine_requests.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>
#include <kernel/panic.h>

#define DIRECT_VRAM_WRITE false

static void hcf(void) {
    for (;;) {
        asm volatile ("hlt");
    }
}

// Debug Function to create a delay loop (not precise, but sufficient for simple timing)
static void delay(uint64_t count) {
    volatile uint64_t i;
    for (i = 0; i < count; i++) {
        asm volatile("nop");
    }
}

// --- KERNEL ENTRY POINT ---
void kmain(void) {
    if (!limine_init_check()) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    screen_init(framebuffer);

    kprint_default_scale("UnknownKernel initialized\n", COLOR_WHITE, DIRECT_VRAM_WRITE);
    kprint_default_scale("Welcome to unknownOS!", COLOR_CYAN, DIRECT_VRAM_WRITE);


    hcf();
}