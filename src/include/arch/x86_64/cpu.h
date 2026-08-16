#pragma once

#include <stdint.h>
#include <stdbool.h>

void enable_interrupts(void);

void disable_interrupts(void);

void hcf(void);

void halt(void);

void halt_sti(void);