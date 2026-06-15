#pragma once

#include <stdint.h>
#include <stdbool.h>

void hcf(void);

void delay(uint64_t count);

bool is_running_on_qemu(void);

void qemu_exit(void);