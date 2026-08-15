#pragma once

#include <stdint.h>
#include <stdbool.h>

bool ps2_wait_input_empty(void);
bool ps2_wait_output_full(void);
void ps2_flush_output_buffer(void);
bool ps2_write_command(uint8_t command);
bool ps2_write_data(uint8_t data);
void ps2_keyboard_enable(void);