#include <arch/x86_64/lapic.h>
#include <arch/x86_64/cpu.h>
#include <kernel/memory/pmm.h>
#include <arch/x86_64/memory.h>
#include <stdint.h>
#include <drivers/screen/screen.h>
#include <klib/mem.h>
#include <kernel/memory/vmm.h>

static uintptr_t lapic_base = 0xFEE00000;

#define P2V(phys) ((void*)((uint64_t)(phys) + get_hhdm_offset()))
#define V2P(virt) ((uint64_t)(virt) - get_hhdm_offset())

uint32_t lapic_read(uint32_t reg) {
    return *(volatile uint32_t *)(lapic_base + reg);
}

void lapic_write(uint32_t reg, uint32_t value) {
    *(volatile uint32_t *)(lapic_base + reg) = value;
}

// Send EOI (End Of Interrupt) to the Local APIC
void lapic_eoi(void) {
    lapic_write(LAPIC_EOI, 0);
}

void lapic_init(void) {
    uint32_t lo, hi;
    
    uint64_t hhdm = get_hhdm_offset();
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(IA32_APIC_BASE_MSR));
    
    uint64_t msr_base = ((uint64_t)hi << 32) | lo;
    uint64_t phys_base = (msr_base & 0xFFFFF000);
    uint64_t virt_base = phys_base + hhdm;

    lapic_base = virt_base;

    // Converti il CR3 (fisico) in indirizzo virtuale prima di passarlo a vmm_map_page
    page_table_t *current_pml4 = (page_table_t *)P2V(get_current_cr3() & PAGE_FRAME_MASK);

    vmm_map_page(current_pml4, virt_base, phys_base,
                 VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE | VMM_FLAG_CACHE_DISABLE);

    if (!(lo & IA32_APIC_BASE_MSR_ENABLE)) {
        lo |= IA32_APIC_BASE_MSR_ENABLE;
        __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(IA32_APIC_BASE_MSR));
    }

    lapic_write(LAPIC_SVR, LAPIC_SVR_ENABLE | 0xFF);

    lapic_write(LAPIC_TPR, 0);
}