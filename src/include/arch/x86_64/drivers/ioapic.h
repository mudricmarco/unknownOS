#pragma once

#include <stdint.h>

#define IOAPIC_IDT_VECTOR 0x21

void ioapic_init_keyboard(uint8_t target_apic_id);