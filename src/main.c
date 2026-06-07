#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <bootloader/limine.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>

// --- LIMINE CONFIGURATION ---
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;


static void hcf(void) {
    for (;;) {
        asm volatile ("hlt");
    }
}

// Micro-funzione di delay software per rallentare QEMU.
static void delay(uint64_t count) {
    volatile uint64_t i;
    for (i = 0; i < count; i++) {
        asm volatile("nop");
    }
}

// --- KERNEL ENTRY POINT ---
void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    screen_init(framebuffer);

    // Messaggi iniziali
    kprint_default_scale("UnknownKernel initialized\n", COLOR_WHITE);
    kprint_default_scale("Welcome to unknownOS!\n", COLOR_CYAN);
    kprint_default_scale("Testing scroll mechanism...\n\n", COLOR_GREEN);
    
    delay(40000000); // Pausa iniziale per leggere il benvenuto

    // Generiamo 120 righe per testare approfonditamente lo scrolling continuo
    for (int i = 1; i <= 120; i++) {
        uint32_t current_color;
        if (i % 3 == 0) {
            current_color = COLOR_WHITE;
        } else if (i % 3 == 1) {
            current_color = COLOR_CYAN;
        } else {
            current_color = COLOR_GREEN;
        }

        // Stampiamo l'intestazione del log (senza lo spazio iniziale fantasma)
        kprint_default_scale("[LOG] Testing line number: ", current_color);
        
        // Scomposizione matematica dei caratteri ASCII per gestire fino a 3 cifre (100-120)
        char centinaia = '0' + (i / 100);
        char decine    = '0' + ((i / 10) % 10);
        char unita     = '0' + (i % 10);

        if (i < 100) {
            // Stringa a due cifre per i numeri < 100 (es. 99\n)
            char num_str[4] = { decine, unita, '\n', '\0' };
            kprint_default_scale(num_str, current_color);
        } else {
            // Stringa a tre cifre per i numeri >= 100 (es. 120\n)
            char num_str[5] = { centinaia, decine, unita, '\n', '\0' };
            kprint_default_scale(num_str, current_color);
        }

        delay(15000000); // Rallenta lo scorrimento visivo in QEMU
    }

    // Segnale di fine test
    kprint_default_scale("\n[SUCCESS] Scroll test completed.", COLOR_CYAN);

    hcf();
}