#pragma once

#include <stdint.h>

#define IA32_APIC_BASE_MSR        0x1B
#define IA32_APIC_BASE_MSR_ENABLE (1 << 11)

// Local APIC offset addresses
#define LAPIC_ID                  0x0020
#define LAPIC_TPR                 0x0080
#define LAPIC_EOI                 0x00B0
#define LAPIC_SVR                 0x00F0
#define LAPIC_TIMER               0x0320
#define LAPIC_TICR                0x0380
#define LAPIC_TCCR                0x0390
#define LAPIC_TDCR                0x03E0

// SVR Flags
#define LAPIC_SVR_ENABLE          (1 << 8)

void lapic_init(void);
void lapic_eoi(void);
uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t value);