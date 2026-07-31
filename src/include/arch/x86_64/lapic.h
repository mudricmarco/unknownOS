#pragma once

#include <stdint.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_ENABLE (1 << 11)

#define LAPIC_ID          0x0020
#define LAPIC_EOI         0x00B0
#define LAPIC_SVR         0x00F0
#define LAPIC_SVR_ENABLE  (1 << 8)

void lapic_init(void);
void lapic_eoi(void);
uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t value);