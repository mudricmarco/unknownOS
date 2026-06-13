#include <kernel/panic.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>
#include <drivers/serial.h>
#include <bootloader/limine_requests.h>
#include <memory/memory.h>
#include <string.h>
#include <stdint.h>

static void hcf(void) {
    for (;;) {
        asm volatile ("hlt");
    }
}

//! Detect if we're running inside QEMU by checking the CPUID signature, for debugging purpose
bool is_running_on_qemu(void) {
    uint32_t eax, ebx, ecx, edx;
    
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(0x40000000));
    
    char signature[13];
    memcpy(signature, &ebx, 4);
    memcpy(signature + 4, &ecx, 4);
    memcpy(signature + 8, &edx, 4);
    signature[12] = '\0';

    return (strcmp(signature, "TCGTCGTCGTCG") == 0 || 
            strcmp(signature, "KVMKVMKVM") == 0);
}

//? Just a fun little function to draw a sad face on the screen during panic, for a bit of personality during kernel panics.
void draw_sad_face(uint32_t color) {
    if (screen_get_fb() == NULL) return;

    int32_t scale = 2;
    int32_t face_x = (int32_t)screen_get_fb()->width / 2;
    int32_t face_y = (int32_t)screen_get_cursor_y() + ((int32_t)screen_get_fb()->height - (int32_t)screen_get_cursor_y()) / 2 - (50 * scale);

    for (int32_t y = -20 * scale; y < 0; y++) {
        for (int32_t x = -40 * scale; x < -20 * scale; x++) screen_put_pixel(face_x + x, face_y + y, color, true);
        for (int32_t x = 20 * scale; x < 40 * scale; x++)   screen_put_pixel(face_x + x, face_y + y, color, true);
    }

    for (int32_t x = -30 * scale; x <= 30 * scale; x++) {
        int32_t y = (x * x) / (30 * scale); 
        
        for(int32_t thick = 0; thick < scale; thick++) {
            screen_put_pixel(face_x + x, face_y + y + (50 * scale) + thick, color, true);
        }
    }
}

void kernel_panic(const char *message) {
    asm volatile ("cli");

    serial_init();
    serial_puts("\n\n!!! KERNEL PANIC !!!\n");
    serial_puts("\n");
    serial_puts(message);
    serial_puts("\n\nSystem halted.");

    if (limine_init_check()) {
        screen_clear(COLOR_BLACK, true);

        kprint_default_scale("\n\n!!! KERNEL PANIC !!!\n", COLOR_RED, true);
        kprint_default_scale("\n", COLOR_WHITE, true);
        kprint_default_scale(message, COLOR_WHITE, true);
        kprint_default_scale("\n\nSystem halted.", COLOR_RED, true);

        //? Draw a sad face for a bit of personality during kernel panics.
        draw_sad_face(COLOR_RED);

    } else if (is_running_on_qemu()) {
        //! If we're running on QEMU, use the QEMU exit mechanism to signal a panic state (only for debugging purposes)
        asm volatile ("outw %0, %1" : : "a"((uint16_t)0x2000), "Nd"((uint16_t)0x604));
    }

    hcf();
}