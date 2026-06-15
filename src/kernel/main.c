#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <bootloader/limine.h>
#include <bootloader/limine_requests.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>
#include <kernel/panic.h>
#include <arch/cpu.h>

#define DIRECT_VRAM_WRITE false

// --- KERNEL ENTRY POINT ---
void kmain(void) {
    if (!limine_init_check()) {
        // If limine init fails, launch a kernel panic in the serial console. 
        // This is a critical failure, as we won't have a framebuffer to display the panic message on the screen.
        kernel_panic("LIMINE initialization failed. No framebuffer available.");
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    screen_init(framebuffer);

    kprint_default_scale("UnknownKernel initialized\n", COLOR_WHITE, DIRECT_VRAM_WRITE);
    kprint_default_scale("Welcome to unknownOS!", COLOR_CYAN, DIRECT_VRAM_WRITE);

    hcf();
}