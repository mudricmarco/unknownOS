#include <arch/x86_64/drivers/ps2.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/io.h>
#endif

bool ps2_wait_input_empty(void) {
    for (uint32_t i = 0; i < 100000; i++) {
        if ((inb(0x64) & 0x02) == 0) {
            return true;
        }
    }

    return false;
}

bool ps2_wait_output_full(void) {
    for (uint32_t i = 0; i < 100000; i++) {
        if (inb(0x64) & 0x01) {
            return true;
        }
    }

    return false;
}

void ps2_flush_output_buffer(void) {
    while (inb(0x64) & 0x01) {
        (void)inb(0x60);
    }
}

bool ps2_write_command(uint8_t command) {
    if (!ps2_wait_input_empty()) {
        return false;
    }

    outb(0x64, command);
    io_wait();
    return true;
}

bool ps2_write_data(uint8_t data) {
    if (!ps2_wait_input_empty()) {
        return false;
    }

    outb(0x60, data);
    io_wait();
    return true;
}

void ps2_keyboard_enable(void) {
    ps2_flush_output_buffer();

    (void)ps2_write_command(0xAD);
    (void)ps2_write_command(0xA7);

    if (ps2_write_command(0x20) && ps2_wait_output_full()) {
        uint8_t command_byte = inb(0x60);
        command_byte |= 0x01;
        command_byte &= ~0x10;
        command_byte |= 0x40;

        (void)ps2_write_command(0x60);
        (void)ps2_write_data(command_byte);
    }

    (void)ps2_write_command(0xAE);
    (void)ps2_write_data(0xF4);
    if (ps2_wait_output_full()) {
        (void)inb(0x60);
    }
}