#include <stdint.h>
#include <arch/x86_64/drivers/ioapic.h>
#include <bootloader/limine_requests.h>
#include <kernel/memory/vmm.h>
#include <arch/x86_64/memory.h>

#define IOAPIC_BASE_PHYSICAL_ADDRESS 0xFEC00000
#define IOAPIC_BASE_VIRTUAL_ADDRESS  (IOAPIC_BASE_PHYSICAL_ADDRESS + hhdm_request.response->offset)
#define IOREGSEL (IOAPIC_BASE_VIRTUAL_ADDRESS + 0x00)
#define IOWIN    (IOAPIC_BASE_VIRTUAL_ADDRESS + 0x10)

#define P2V(phys) ((void*)((uint64_t)(phys) + hhdm_request.response->offset))
#define V2P(virt) ((uint64_t)(virt) - hhdm_request.response->offset())

static void ioapic_write(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)(IOREGSEL) = reg;
    *(volatile uint32_t*)(IOWIN) = value;
}

void ioapic_init_keyboard(uint8_t target_apic_id) {
    page_table_t *current_pml4 = (page_table_t *)P2V(get_current_cr3() & PAGE_FRAME_MASK);

    vmm_map_page(current_pml4, IOAPIC_BASE_VIRTUAL_ADDRESS, IOAPIC_BASE_PHYSICAL_ADDRESS,
                 VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE | VMM_FLAG_CACHE_DISABLE);

    uint8_t irq = 1;
    ioapic_write(0x10 + 2 * irq, IOAPIC_IDT_VECTOR);
    ioapic_write(0x10 + 2 * irq + 1, ((uint32_t)target_apic_id) << 24);
}