//TODO: Change this file to use the kernel's paging system instead of manually mapping the LAPIC MMIO region.

#include <arch/x86_64/lapic.h>
#include <arch/x86_64/cpu.h>
#include <kernel/memory/pmm.h>
#include <arch/x86_64/memory.h>
#include <stdint.h>
#include <drivers/screen/screen.h>
#include <klib/mem.h>

static uintptr_t lapic_base = 0xFEE00000;

#define P2V(phys) ((void*)((uint64_t)(phys) + get_hhdm_offset()))
#define V2P(virt) ((uint64_t)(virt) - get_hhdm_offset())

// Temporary function to map a physical address to a virtual address in the HHDM (Higher Half Direct Mapping) region
//TODO: Remove after the kernel's paging system is fully implemented and the LAPIC is mapped correctly.
void map_mmio_page(uint64_t virt_addr, uint64_t phys_addr) {
    uint64_t cr3 = get_current_cr3();
    
    uint64_t* pml4 = (uint64_t*)P2V(cr3 & ~0xFFF);

    uint64_t pml4_idx = (virt_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt_addr >> 12) & 0x1FF;

    if (!(pml4[pml4_idx] & 1)) { // Bit 0 = Present
        void* new_table = pmm_alloc_page();
        memset(P2V(new_table), 0, 4096);
        pml4[pml4_idx] = (uint64_t)new_table | 0x03; // Present + Writable
    }
    uint64_t* pdpt = (uint64_t*)P2V(pml4[pml4_idx] & ~0xFFF);

    if (!(pdpt[pdpt_idx] & 1)) {
        void* new_table = pmm_alloc_page();
        memset(P2V(new_table), 0, 4096);
        pdpt[pdpt_idx] = (uint64_t)new_table | 0x03;
    }
    uint64_t* pd = (uint64_t*)P2V(pdpt[pdpt_idx] & ~0xFFF);

    if (!(pd[pd_idx] & 1)) {
        void* new_table = pmm_alloc_page();
        memset(P2V(new_table), 0, 4096);
        pd[pd_idx] = (uint64_t)new_table | 0x03;
    }
    uint64_t* pt = (uint64_t*)P2V(pd[pd_idx] & ~0xFFF);

    pt[pt_idx] = (phys_addr & ~0xFFF) | 0x03 | (1 << 4);

    __asm__ volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

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

    map_mmio_page(virt_base, phys_base);

    if (!(lo & IA32_APIC_BASE_MSR_ENABLE)) {
        lo |= IA32_APIC_BASE_MSR_ENABLE;
        __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(IA32_APIC_BASE_MSR));
    }

    lapic_write(LAPIC_SVR, LAPIC_SVR_ENABLE | 0xFF);

    lapic_write(LAPIC_TPR, 0);
}