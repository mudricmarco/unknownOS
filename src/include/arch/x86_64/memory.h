#pragma once

#include <stdint.h>

uint64_t get_current_cr3(void);

void write_cr3(uint64_t phys_addr);