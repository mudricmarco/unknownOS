#include "drivers/screen/screen.h"
#include <stdint.h>
#include <stddef.h>
#include <bootloader/limine_requests.h>
#include <kernel/memory/pmm.h>
#include <kernel/panic.h>
#include <klib/math.h>

void init_physical_memory(void) {
    if (memmap_request.response == NULL || memmap_request.response->entry_count <= 0) {
        kernel_panic("Failed to initialize physical memory: No valid memory map provided by the bootloader.");
    }
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