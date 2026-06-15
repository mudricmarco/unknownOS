#include <kernel/panic.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>
#include <drivers/serial.h>
#include <bootloader/limine_requests.h>
#include <klib/mem.h>
#include <arch/cpu.h>
#include <stdint.h>

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
        qemu_exit();
    }

    hcf();
}