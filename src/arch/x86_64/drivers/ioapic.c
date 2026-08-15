#include <stdint.h>
#include <arch/x86_64/drivers/ioapic.h>
#include <bootloader/limine_requests.h>

#define IOAPIC_BASE_PHYSICAL_ADDRESS 0xFEC00000
#define IOAPIC_BASE_VIRTUAL_ADDRESS  (IOAPIC_BASE_PHYSICAL_ADDRESS + hhdm_request.response->offset)
#define IOREGSEL (IOAPIC_BASE_VIRTUAL_ADDRESS + 0x00)
#define IOWIN    (IOAPIC_BASE_VIRTUAL_ADDRESS + 0x10)

extern void map_mmio_page(uint64_t virt_addr, uint64_t phys_addr);

static void ioapic_write(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)(IOREGSEL) = reg;
    *(volatile uint32_t*)(IOWIN) = value;
}

void ioapic_init_keyboard(uint8_t target_apic_id) {
    map_mmio_page(IOAPIC_BASE_VIRTUAL_ADDRESS, IOAPIC_BASE_PHYSICAL_ADDRESS);

    uint8_t irq = 1;
    ioapic_write(0x10 + 2 * irq, IOAPIC_IDT_VECTOR);
    ioapic_write(0x10 + 2 * irq + 1, ((uint32_t)target_apic_id) << 24);
}