#include <kernel/panic.h>
#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>
#include <drivers/serial.h>
#include <bootloader/limine_requests.h>
#include <klib/mem.h>
#include <stdint.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#endif

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
    disable_interrupts();
    serial_puts("\n\n!!! KERNEL PANIC !!!\n");
    serial_puts("\n");
    serial_puts(message);
    serial_puts("\n\nSystem halted.");

    screen_clear(COLOR_BLACK, true);

    kprintf(COLOR_WHITE, 2, true, "\n\n%C!!! KERNEL PANIC !!!\n\n%C%s\n\n%CSystem halted.", 
            COLOR_RED, COLOR_WHITE, message, COLOR_RED);

    //? Draw a sad face for a bit of personality during kernel panics.
    draw_sad_face(COLOR_RED);


    hcf();
}

void kernel_panic_detailed(struct registers* regs, const char* message) {
    disable_interrupts();

    char buffer[2048];

    create_string_buf(
        buffer, 
        sizeof(buffer),
        "unknownOS has encountered a fatal error and must be shut down.\n"
        "Detail: %s (Vector: %d, Error Code: 0x%x)\n\n"
        "--- STATE DUMP (REGISTERS) ---\n"
        "RIP: 0x%x    CS:  0x%x    RFLAGS: 0x%x\n"
        "RSP: 0x%x    SS:  0x%x\n\n"
        "RAX: 0x%x    RBX: 0x%x    RCX:    0x%x    RDX: 0x%x\n"
        "RSI: 0x%x    RDI: 0x%x    RBP:    0x%x\n"
        "R8:  0x%x    R9:  0x%x    R10:    0x%x    R11: 0x%x\n"
        "R12: 0x%x    R13: 0x%x    R14:    0x%x    R15: 0x%x\n\n"
        "--------------------------------------------------------------------------------\n"
        "The system has been halted. Please reboot your hardware manually.\n",
        message, regs->int_no, regs->error_code,
        regs->rip, regs->cs, regs->rflags, regs->rsp, regs->ss,
        regs->rax, regs->rbx, regs->rcx, regs->rdx,
        regs->rsi, regs->rdi, regs->rbp,
        regs->r8,  regs->r9,  regs->r10, regs->r11,
        regs->r12, regs->r13, regs->r14, regs->r15
    );

    kernel_panic(buffer);
}