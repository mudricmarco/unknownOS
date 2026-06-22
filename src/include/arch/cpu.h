#pragma once

#include <stdint.h>
#include <stdbool.h>

void enable_interrupts(void);

void disable_interrupts(void);

void hcf(void);

void delay(uint64_t count);