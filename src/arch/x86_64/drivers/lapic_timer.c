//TODO: Add mode choice (periodic, one-shot, TSC deadline)
//TODO: Add support for TSC deadline timer

#include <arch/x86_64/idt.h>
#include <arch/x86_64/drivers/lapic_timer.h>
#include <arch/x86_64/lapic.h>
#include <kernel/timer/timer.h>
#include <arch/x86_64/registers.h>
#include <arch/x86_64/io.h>

#include <drivers/screen/screen.h>
#include <drivers/acpi.h>

static volatile uint64_t lapic_timer_ticks = 0;
static uint32_t selected_divider = LAPIC_TIMER_DIV_16;

#define ACPI_PM_FREQ 3579545

static void acpi_pm_delay_ms(uint16_t pm_port, uint32_t ms) {
    uint32_t target_ticks = (ms * ACPI_PM_FREQ) / 1000;
    uint32_t start = inl(pm_port) & 0x00FFFFFF;
    uint32_t elapsed = 0;

    while (elapsed < target_ticks) {
        uint32_t current = inl(pm_port) & 0x00FFFFFF;
        
        if (current >= start) {
            elapsed += (current - start);
        } else {
            elapsed += ((0x00FFFFFF - start) + current);
        }
        
        start = current;
        __asm__ volatile("pause");
    }
}

static void lapic_timer_init_callback(uint32_t frequency) {
    if (frequency == 0) frequency = 1000;

    uint16_t pm_port = get_acpi_pm_timer_port();
    uint32_t initial_count = 0;

    if (pm_port != 0) {
        lapic_write(LAPIC_TDCR, selected_divider);

        lapic_write(LAPIC_TIMER, LAPIC_TIMER_VECTOR);

        lapic_write(LAPIC_TICR, 0xFFFFFFFF);

        acpi_pm_delay_ms(pm_port, 10);

        uint32_t current_count = lapic_read(LAPIC_TCCR);
        uint32_t elapsed_ticks = 0xFFFFFFFF - current_count;

        uint32_t ticks_per_ms = elapsed_ticks / 10;
        initial_count = ticks_per_ms * (1000 / frequency);
    }

    if (initial_count == 0) {
        initial_count = 100000000 / frequency;
    }

    lapic_write(LAPIC_TDCR, selected_divider);
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_VECTOR | LAPIC_TIMER_PERIODIC);
    lapic_write(LAPIC_TICR, initial_count);
}

static uint64_t lapic_timer_get_ticks(void) {
    return lapic_timer_ticks;
}

// IDT handler for the LAPIC timer interrupt
void lapic_timer_irq_handler(struct registers* regs) {
    (void)regs;
    lapic_timer_ticks++;
    lapic_eoi();
}

static timer_device_t lapic_timer_dev = {
    .name = "lapic_timer",
    .init = lapic_timer_init_callback,
    .get_ticks = lapic_timer_get_ticks
};

void lapic_timer_device_init(uint32_t divider) {
    selected_divider = divider;
    timer_register(&lapic_timer_dev, 1000);
}