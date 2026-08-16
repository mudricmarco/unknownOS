#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 4096ULL

#define PAGE_FRAME_MASK 0x000FFFFFFFFFF000ULL

#define VMM_FLAG_CACHE_DISABLE (1ULL << 4)

#define VMM_FLAG_PRESENT (1ULL << 0)
#define VMM_FLAG_WRITABLE (1ULL << 1)
#define VMM_FLAG_USER (1ULL << 2)
#define VMM_FLAG_NO_EXECUTE (1ULL << 63)

#define PML4_INDEX(virt_addr) ((virt_addr >> 39) & 0x1FF)
#define PDPT_INDEX(virt_addr) ((virt_addr >> 30) & 0x1FF)
#define PD_INDEX(virt_addr) ((virt_addr >> 21) & 0x1FF)
#define PT_INDEX(virt_addr) ((virt_addr >> 12) & 0x1FF)

typedef struct {
    uint64_t entries[512];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

bool vmm_init(void);
bool vmm_map_page(page_table_t *pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
bool vmm_unmap_page(page_table_t *pml4, uint64_t virt_addr);
void vmm_map_hhdm(page_table_t *kernel_pml4);
void vmm_map_kernel(page_table_t *pml4);
void vmm_map_framebuffer(page_table_t *kernel_pml4);