//! This is still a very basic (and not very efficient) physical memory manager. I should really implement a better version.
//! Still, for now, this is enough to get the kernel up and running. I will improve it later.
//! (I still hate it btw)
//TODO: Improve the physical memory manager
#include "bootloader/limine.h"
#include "drivers/screen/screen.h"
#include <stdint.h>
#include <stddef.h>
#include <bootloader/limine_requests.h>
#include <kernel/memory/pmm.h>
#include <kernel/panic.h>
#include <klib/math.h>
#include <arch/memory.h>

#define P2V(phys) ((void*)((uint64_t)(phys) + hhdm_offset))
#define V2P(virt) ((uint64_t)(virt) - hhdm_offset)

#define ALIGN_UP(val, align)   (((val) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(val, align) ((val) & ~((align) - 1))

bool physical_memory_initialized = false;

uint64_t hhdm_offset = 0;

uint8_t* pmm_bitmap = NULL;
uint64_t pmm_bitmap_size = 0;
uint64_t pmm_total_pages = 0;

uint64_t pmm_last_free_index = 0;

uint64_t _kernel_start_phys = 0;
uint64_t _kernel_end_phys = 0;

static uint64_t pmm_bootloader_cr3 = 0;

bool is_physical_memory_initialized(void) {
    return physical_memory_initialized;
}

uint64_t get_kernel_start_phys(void) {
    return _kernel_start_phys;
}

uint64_t get_kernel_end_phys(void) {
    return _kernel_end_phys;
}

uint64_t get_hhdm_offset(void) {
    return hhdm_offset;
}

static inline void bitmap_set_page(uint64_t page) {
    pmm_bitmap[page / 8] |= (1 << (page % 8));
}

static inline void bitmap_clear_page(uint64_t page) {
    pmm_bitmap[page / 8] &= ~(1 << (page % 8));
}

static inline void pmm_validate_bootloader_requests(void) {
    if (memmap_request.response == NULL || memmap_request.response->entry_count == 0) {
        kernel_panic("Failed to initialize physical memory: No valid memory map provided by the bootloader.");
    }
    if(kernel_address_request.response == NULL || kernel_address_request.response->physical_base == 0) {
        kernel_panic("Failed to initialize physical memory: No valid kernel address provided by the bootloader.");
    }
    if(hhdm_request.response == NULL) {
        kernel_panic("Failed to initialize physical memory: No valid HHDM offset provided by the bootloader.");
    }
}

static inline void pmm_calculate_kernel_bounds(struct limine_memmap_response *memmap) {
    // -- Calculate Kernel start and end physical addresses, and HHDM offset
    hhdm_offset = hhdm_request.response->offset;
    _kernel_start_phys = kernel_address_request.response->physical_base;

    uint64_t kernel_size = 0;

    // Limine does not directly expose the loaded kernel physical end
    // For now we assume the EXECUTABLE_AND_MODULES entry corresponds to the kernel image.
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES && entry->base == _kernel_start_phys) {
            kernel_size = entry->length;
            break;
        }
    }

    if (kernel_size == 0) {
        kernel_panic("Failed to initialize physical memory: Kernel size is zero. No valid executable memory region found in the memory map.");
    }

    _kernel_end_phys = _kernel_start_phys + kernel_size;
}

static inline uint64_t pmm_find_highest_address(struct limine_memmap_response *memmap) {
    uint64_t highest_address = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        uint64_t entry_end = entry->base + entry->length;
        if (entry_end > highest_address) {
            highest_address = entry_end;
        }
    }
    return highest_address;
}

static inline uint64_t pmm_allocate_bitmap_placeholder(struct limine_memmap_response *memmap) {
    uint64_t bitmap_phys_addr = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= pmm_bitmap_size) {
            bitmap_phys_addr = entry->base;
            break;
        }
    }

    if (bitmap_phys_addr == 0) {
        kernel_panic("Failed to initialize physical memory: Not enough continuous usable memory to allocate the PMM bitmap.");
    }
    return bitmap_phys_addr;
}

static inline void pmm_init_bitmap_metadata(struct limine_memmap_response *memmap) {
    // -- Configure the memory bitmap --
    uint64_t highest_address = pmm_find_highest_address(memmap);

    pmm_total_pages = (highest_address + 4095) / 4096;
    pmm_bitmap_size = pmm_total_pages / 8;
    if (pmm_total_pages % 8 != 0) {
        pmm_bitmap_size++;
    }

    uint64_t bitmap_phys_addr = pmm_allocate_bitmap_placeholder(memmap);
    pmm_bitmap = (uint8_t*)P2V(bitmap_phys_addr);

    uint64_t* bitmap64 = (uint64_t*)pmm_bitmap;
    uint64_t size64 = pmm_bitmap_size / 8;

    for (uint64_t i = 0; i < size64; i++) {
        bitmap64[i] = 0xFFFFFFFFFFFFFFFF;
    }

    for (uint64_t i = size64 * 8; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFF;
    }
}

static inline void pmm_map_usable_memory_regions(struct limine_memmap_response *memmap) {
    uint64_t* bitmap64 = (uint64_t*)pmm_bitmap;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t aligned_base = ALIGN_UP(entry->base, 4096);
            uint64_t aligned_end = ALIGN_DOWN(entry->base + entry->length, 4096);

            if (aligned_base >= aligned_end) continue;

            uint64_t start_page = aligned_base / 4096;
            uint64_t end_page = aligned_end / 4096;

            uint64_t page = start_page;

            uint64_t next_aligned_page = ALIGN_UP(start_page, 64);
            if (next_aligned_page > end_page) {
                next_aligned_page = end_page;
            }

            for (; page < next_aligned_page; page++) {
                bitmap_clear_page(page);
            }
            
            uint64_t bulk_end_page = ALIGN_DOWN(end_page, 64);
            for (; page < bulk_end_page; page += 64) {
                bitmap64[page / 64] = 0x0000000000000000;
            }

            for (; page < end_page; page++) {
                bitmap_clear_page(page);
            }
        }
    }
}

static inline void pmm_protect_critical_structures(uint64_t bitmap_phys_addr) {
    uint64_t kernel_start_page = ALIGN_DOWN(_kernel_start_phys, 4096) / 4096;
    uint64_t kernel_end_page = ALIGN_UP(_kernel_end_phys, 4096) / 4096;
    for (uint64_t page = kernel_start_page; page < kernel_end_page; page++) {
        bitmap_set_page(page);
    }

    uint64_t bitmap_start_page = ALIGN_DOWN(bitmap_phys_addr, 4096) / 4096;
    uint64_t bitmap_end_page = ALIGN_UP(bitmap_phys_addr + pmm_bitmap_size, 4096) / 4096;

    for (uint64_t page = bitmap_start_page; page < bitmap_end_page; page++) {
        bitmap_set_page(page);
    }
}

void init_physical_memory(void) {
    pmm_validate_bootloader_requests();
    
    struct limine_memmap_response *memmap = memmap_request.response;

    pmm_calculate_kernel_bounds(memmap);
    pmm_init_bitmap_metadata(memmap);
    pmm_map_usable_memory_regions(memmap);
    
    uint64_t bitmap_phys_addr = V2P(pmm_bitmap);
    pmm_protect_critical_structures(bitmap_phys_addr);

    pmm_bootloader_cr3 = get_current_cr3();

    physical_memory_initialized = true;
}

size_t get_total_memory_size(void) {
    size_t total_mem = 0;
    struct limine_memmap_response *memmap = memmap_request.response;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_mem += entry->length;
        }
    }
    
    return total_mem;
}

void* pmm_alloc_page(void) {
    if (!physical_memory_initialized) {
        kernel_panic("Attempted to allocate a physical page before the physical memory manager was initialized");
    }

    uint64_t* bitmap64 = (uint64_t*)pmm_bitmap;
    uint64_t size64 = pmm_bitmap_size / 8;

    for (uint64_t i = pmm_last_free_index; i < size64; i++) {
        if (bitmap64[i] != 0xFFFFFFFFFFFFFFFF) {
            
            uint64_t bit = __builtin_ctzll(~bitmap64[i]);
            
            uint64_t page = i * 64 + bit;
            if (page >= pmm_total_pages) return NULL;

            bitmap_set_page(page);
            pmm_last_free_index = i;
            return (void*)(page * 4096);
        }
    }
    return NULL;
}

bool pmm_free_page(void* phys_addr) {
    if (!physical_memory_initialized) {
        return false;
    }

    uint64_t addr = (uint64_t)phys_addr;
    
    if (addr % 4096 != 0) {
        kernel_panic("Attempted to free a physical page that is not aligned to 4KB");
    }

    uint64_t page = addr / 4096;

    if (page >= pmm_total_pages) {
        kernel_panic("Attempted to free a physical page that is out of bounds of the physical memory bitmap");
    }

    if (!(pmm_bitmap[page / 8] & (1 << (page % 8)))) {
        kernel_panic("Attempted to free a physical page that is already free");
    }

    bitmap_clear_page(page);

    uint64_t index64 = page / 64;
    if (index64 < pmm_last_free_index) {
        pmm_last_free_index = index64;
    }

    return true;
}

// Reclaim memory marked as reclaimable by the bootloader (for when i change the cr3 page table)
void pmm_reclaim_bootloader_memory(void) {
    if (!physical_memory_initialized) {
        return;
    }

    if (get_current_cr3() == pmm_bootloader_cr3) {
        kernel_panic("Safety Violation: Attempted to reclaim bootloader memory while its page tables are still active in CR3");
    }

    struct limine_memmap_response *memmap = memmap_request.response;
    
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            uint64_t start_page = entry->base / 4096;
            uint64_t end_page = (entry->base + entry->length) / 4096;

            for (uint64_t page = start_page; page < end_page; page++) {
                bitmap_clear_page(page);
            }
        }
    }
    
    pmm_last_free_index = 0;
}