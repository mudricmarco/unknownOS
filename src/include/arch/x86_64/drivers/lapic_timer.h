#pragma once

#include <kernel/timer/timer.h>
#include <stdint.h>

#include <arch/x86_64/registers.h>

#define LAPIC_TIMER_VECTOR 0x20

// LVT Flags
#define LAPIC_TIMER_ONESHOT       (0 << 17)
#define LAPIC_TIMER_PERIODIC      (1 << 17)
#define LAPIC_TIMER_TSC_DEADLINE  (2 << 17)
#define LAPIC_TIMER_MASKED        (1 << 16)

// Dividers
#define LAPIC_TIMER_DIV_1         0x0B
#define LAPIC_TIMER_DIV_2         0x00
#define LAPIC_TIMER_DIV_4         0x01
#define LAPIC_TIMER_DIV_8         0x02
#define LAPIC_TIMER_DIV_16        0x03
#define LAPIC_TIMER_DIV_32        0x08
#define LAPIC_TIMER_DIV_64        0x09
#define LAPIC_TIMER_DIV_128       0x0A

void lapic_timer_device_init(uint32_t divider);
void lapic_timer_irq_handler(struct registers* regs);