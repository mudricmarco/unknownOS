#include <bootloader/limine.h>
#include <kernel/memory/vmm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/pmm.h>
#include <bootloader/limine_requests.h>
#include <klib/mem.h>
#include <kernel/panic.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#include <arch/x86_64/memory.h>
#endif

#ifdef CONFIG_ARCH_X86_64
#define IA32_EFER_MSR 0xC0000080
#define IA32_EFER_NXE (1ULL << 11)

static void enable_nxe(void) {
    uint32_t lo, hi;
    uint64_t efer;

    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(IA32_EFER_MSR));
    efer = ((uint64_t)hi << 32) | lo;
    efer |= IA32_EFER_NXE;
    lo = (uint32_t)efer;
    hi = (uint32_t)(efer >> 32);
    __asm__ volatile("wrmsr" :: "a"(lo), "d"(hi), "c"(IA32_EFER_MSR));
}
#endif

#define P2V(phys) ((void*)((uint64_t)(phys) + get_hhdm_offset()))
#define V2P(virt) ((uint64_t)(virt) - get_hhdm_offset())

#define LARGE_PAGE_SIZE (2 * 1024 * 1024)
#define VMM_FLAG_PS (1ULL << 7)

// Map 2MB if available
static bool vmm_map_range(page_table_t *pml4, uint64_t virt_start, uint64_t phys_start, uint64_t length, uint64_t flags) {
    uint64_t virt = virt_start;
    uint64_t phys = phys_start;
    uint64_t end = virt_start + length;

    uint64_t table_flags = VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE;
    if (flags & VMM_FLAG_USER) {
        table_flags |= VMM_FLAG_USER;
    }

    while (virt < end) {
        if ((virt % LARGE_PAGE_SIZE) == 0 && (phys % LARGE_PAGE_SIZE) == 0 && (end - virt) >= LARGE_PAGE_SIZE) {
            uint64_t pml4_index = PML4_INDEX(virt);
            uint64_t pdpt_index = PDPT_INDEX(virt);
            uint64_t pd_index = PD_INDEX(virt);
            page_table_t *pdpt, *pd;

            // Get or create PDPT
            if (pml4->entries[pml4_index] & VMM_FLAG_PRESENT) {
                pdpt = (page_table_t *)P2V(pml4->entries[pml4_index] & PAGE_FRAME_MASK);
            } else {
                uint64_t pdpt_phys = (uint64_t)pmm_alloc_page();
                if (!pdpt_phys) return false;
                pdpt = (page_table_t *)P2V(pdpt_phys);
                memset(pdpt, 0, PAGE_SIZE);
                pml4->entries[pml4_index] = pdpt_phys | table_flags;
            }

            // Get or create PD
            if (pdpt->entries[pdpt_index] & VMM_FLAG_PRESENT) {
                pd = (page_table_t *)P2V(pdpt->entries[pdpt_index] & PAGE_FRAME_MASK);
            } else {
                uint64_t pd_phys = (uint64_t)pmm_alloc_page();
                if (!pd_phys) return false;
                pd = (page_table_t *)P2V(pd_phys);
                memset(pd, 0, PAGE_SIZE);
                pdpt->entries[pdpt_index] = pd_phys | table_flags;
            }

            // Set PD entry with PS bit for a 2 MiB mapping
            pd->entries[pd_index] = (phys & PAGE_FRAME_MASK) | flags | VMM_FLAG_PRESENT | VMM_FLAG_PS;

            virt += LARGE_PAGE_SIZE;
            phys += LARGE_PAGE_SIZE;
        } else {
            // Fallback to mapping a single 4 KiB page
            if (!vmm_map_page(pml4, virt, phys, flags)) return false;
            virt += PAGE_SIZE;
            phys += PAGE_SIZE;
        }
    }

    return true;
}

// Get symbols from the linker
extern uint8_t __requests_start[], __requests_end[];
extern uint8_t __text_start[],     __text_end[];
extern uint8_t __rodata_start[],   __rodata_end[];
extern uint8_t __data_start[],     __data_end[];

bool vmm_map_page(page_table_t *pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);

    page_table_t *pdpt, *pd, *pt;
    
    // Get or create PDPT
    if (pml4->entries[pml4_index] & VMM_FLAG_PRESENT) {
        pdpt = (page_table_t *)P2V(pml4->entries[pml4_index] & PAGE_FRAME_MASK); // Get the virtual address of the PDPT without the flags
    }
    else {
        uint64_t pdpt_phys = (uint64_t)pmm_alloc_page();
        if(!pdpt_phys) {
            return false;
        }
        pdpt = (page_table_t *)P2V(pdpt_phys);
        memset(pdpt, 0, PAGE_SIZE);

        pml4->entries[pml4_index] = pdpt_phys | VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE | VMM_FLAG_USER;
    }

    // Get or create PD
    if (pdpt->entries[pdpt_index] & VMM_FLAG_PRESENT) {
        pd = (page_table_t *)P2V(pdpt->entries[pdpt_index] & PAGE_FRAME_MASK); // Get the virtual address of the PD without the flags
    }
    else {
        uint64_t pd_phys = (uint64_t)pmm_alloc_page();
        if(!pd_phys) {
            return false;
        }
        pd = (page_table_t *)P2V(pd_phys);
        memset(pd, 0, PAGE_SIZE);

        pdpt->entries[pdpt_index] = pd_phys | VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE | VMM_FLAG_USER;
    }

    // Get or create PT
    if (pd->entries[pd_index] & VMM_FLAG_PRESENT) {
        pt = (page_table_t *)P2V(pd->entries[pd_index] & PAGE_FRAME_MASK); // Get the virtual address of the PT without the flags
    }
    else {
        uint64_t pt_phys = (uint64_t)pmm_alloc_page();
        if(!pt_phys) {
            return false;
        }
        pt = (page_table_t *)P2V(pt_phys);
        memset(pt, 0, PAGE_SIZE);

        pd->entries[pd_index] = pt_phys | VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE | VMM_FLAG_USER;
    }

    // Now map the physical address to the virtual address in the PT
    pt->entries[pt_index] = (phys_addr & PAGE_FRAME_MASK) | flags | VMM_FLAG_PRESENT;

    // Flush the TLB for the virtual address if it's the same cr3
    if(V2P(pml4) == get_current_cr3()) {
        flush_tlb(virt_addr);
    }

    return true;
}

bool vmm_unmap_page(page_table_t *pml4, uint64_t virt_addr) {
    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);

    page_table_t *pdpt, *pd, *pt;

    if (!(pml4->entries[pml4_index] & VMM_FLAG_PRESENT)) {
        return false;
    }
    pdpt = (page_table_t *)P2V(pml4->entries[pml4_index] & PAGE_FRAME_MASK);

    if (!(pdpt->entries[pdpt_index] & VMM_FLAG_PRESENT)) {
        return false;
    }
    pd = (page_table_t *)P2V(pdpt->entries[pdpt_index] & PAGE_FRAME_MASK);

    if (!(pd->entries[pd_index] & VMM_FLAG_PRESENT)) {
        return false;
    }
    pt = (page_table_t *)P2V(pd->entries[pd_index] & PAGE_FRAME_MASK);

    if (!(pt->entries[pt_index] & VMM_FLAG_PRESENT)) {
        return false;
    }
    pt->entries[pt_index] = 0;

    if (V2P(pml4) == get_current_cr3()) {
        flush_tlb(virt_addr);
    }

    return true;
}

void vmm_map_hhdm(page_table_t *kernel_pml4) {
    if (!memmap_request.response) {
        kernel_panic("Limine memory map not available");
    }
    struct limine_memmap_response *memmap = memmap_request.response;

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        uint64_t phys_start = entry->base & PAGE_FRAME_MASK;
        uint64_t phys_end = (entry->base + entry->length + 0xFFF) & PAGE_FRAME_MASK;
        uint64_t length = phys_end - phys_start;
        uint64_t virt_start = (uint64_t)P2V(phys_start);

        if (!vmm_map_range(kernel_pml4, virt_start, phys_start, length, VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE)) {
            kernel_panic("vmm_map_hhdm: failed to map HHDM range");
        }
    }
}

static void vmm_map_section(page_table_t *pml4, uint8_t *start, uint8_t *end, uint64_t flags) {
    if (!kernel_address_request.response) {
        kernel_panic("Limine kernel address not available");
    }
    struct limine_executable_address_response *exec_addr = kernel_address_request.response;

    uint64_t virt_start = (uint64_t)start & PAGE_FRAME_MASK;
    uint64_t virt_end = ((uint64_t)end + 0xFFF) & PAGE_FRAME_MASK;
    uint64_t length = virt_end - virt_start;
    uint64_t phys_start = exec_addr->physical_base + (virt_start - exec_addr->virtual_base);

    if (!vmm_map_range(pml4, virt_start, phys_start, length, flags)) {
        kernel_panic("vmm_map_section: failed to map kernel section");
    }
}

void vmm_map_kernel(page_table_t *pml4) {
    vmm_map_section(pml4, __requests_start, __requests_end, VMM_FLAG_PRESENT | VMM_FLAG_NO_EXECUTE);
    vmm_map_section(pml4, __text_start, __text_end, VMM_FLAG_PRESENT);
    vmm_map_section(pml4, __rodata_start, __rodata_end, VMM_FLAG_PRESENT | VMM_FLAG_NO_EXECUTE);
    vmm_map_section(pml4, __data_start, __data_end, VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE | VMM_FLAG_NO_EXECUTE);
}


void vmm_map_framebuffer(page_table_t *kernel_pml4) {
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    if (!fb) return;

    uint64_t fb_virt = (uint64_t)fb->address;
    uint64_t fb_size = fb->pitch * fb->height;

    uint64_t fb_phys = V2P(fb_virt);

    uint64_t virt_start = fb_virt & PAGE_FRAME_MASK;
    uint64_t virt_end = (fb_virt + fb_size + 0xFFF) & PAGE_FRAME_MASK;
    uint64_t phys_start = fb_phys & PAGE_FRAME_MASK;
    uint64_t length = virt_end - virt_start;

    if (!vmm_map_range(kernel_pml4, virt_start, phys_start, length, VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE)) {
        kernel_panic("vmm_map_framebuffer: failed to map framebuffer");
    }
}

bool vmm_init(void) {
#ifdef CONFIG_ARCH_X86_64
    enable_nxe();
#endif

    uint64_t kernel_pml4_phys = (uint64_t)pmm_alloc_page();
    if (!kernel_pml4_phys) {
        return false;
    }

    page_table_t *kernel_pml4 = (page_table_t *)P2V(kernel_pml4_phys);
    memset(kernel_pml4, 0, PAGE_SIZE);

    vmm_map_hhdm(kernel_pml4);
    vmm_map_kernel(kernel_pml4);
    vmm_map_framebuffer(kernel_pml4);

    write_cr3(kernel_pml4_phys);

    return true;
}