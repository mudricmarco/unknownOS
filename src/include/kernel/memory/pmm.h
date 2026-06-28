#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

bool is_physical_memory_initialized(void);
void init_physical_memory(void);

size_t get_total_memory_size(void);
uint64_t get_kernel_start_phys(void);
uint64_t get_kernel_end_phys(void);
uint64_t get_hhdm_offset(void);

void* pmm_alloc_page(void);
void* pmm_alloc_pages(size_t count);

bool pmm_free_page(void* page);
bool pmm_free_pages(void* phys_addr, size_t count);

void pmm_reclaim_bootloader_memory(void);