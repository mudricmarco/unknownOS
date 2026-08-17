//TODO: Implement a more efficient heap allocator, possibly using a buddy allocator or slab allocator for better performance and reduced fragmentation.
//TODO: Add protection against overwriting the heap boundaries and implement a mechanism to detect memory corruption.

#include <kernel/memory/heap.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <klib/list.h>
#include <klib/mem.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/memory.h>
#endif

#define P2V(phys) ((void*)((uint64_t)(phys) + get_hhdm_offset()))
#define V2P(virt) ((uint64_t)(virt) - get_hhdm_offset())

LIST_HEAD(free_list);

extern uint8_t __heap_start[];

static uint64_t heap_current_virtual = 0;

void heap_init(void) {
    uint64_t heap_start_vaddr = ((uint64_t)__heap_start + PAGE_SIZE - 1) & PAGE_FRAME_MASK;
    heap_current_virtual = heap_start_vaddr;
}

static bool expand_heap(size_t minimum_size) {
    size_t total_needed = minimum_size + HEADER_SIZE;
    size_t pages_needed = (total_needed + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t bytes_allocated = pages_needed * PAGE_SIZE;

    block_header_t *new_block = (block_header_t *)heap_current_virtual;

    page_table_t *pml4 = (page_table_t *)P2V(get_current_cr3());

    for (size_t i = 0; i < pages_needed; i++) {
        uint64_t phys = (uint64_t)pmm_alloc_page();
        if (!phys) {
            return false;
        }

        if (!vmm_map_page(pml4, heap_current_virtual + (i * PAGE_SIZE), phys, VMM_FLAG_PRESENT | VMM_FLAG_WRITABLE)) {
            return false;
        }
    }

    heap_current_virtual += bytes_allocated;

    new_block->size = bytes_allocated - HEADER_SIZE;
    new_block->is_free = true;
    
    list_add_tail(&new_block->elem, &free_list);

    return true;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    size = (size + 15) & ~15; // 16 Byte allignment

    block_header_t *curr;

    list_for_each_entry(curr, &free_list, elem) {
        if (curr->size >= size) {
            list_del(&curr->elem);

            if (curr->size >= size + HEADER_SIZE + 16) {
                block_header_t *remainder = (block_header_t *)((uint8_t *)curr + HEADER_SIZE + size);
                remainder->size = curr->size - size - HEADER_SIZE;
                remainder->is_free = true;

                list_add(&remainder->elem, &free_list);

                curr->size = size;
            }

            curr->is_free = false;
            return (void *)((uint8_t *)curr + HEADER_SIZE);
        }
    }

    if (!expand_heap(size)) {
        return NULL;
    }

    return kmalloc(size);
}

void kfree(void *ptr) {
    if (!ptr) {
        return;
    }

    block_header_t *block = (block_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    
    if (block->is_free) {
        return;
    }

    block->is_free = true;

    list_add(&block->elem, &free_list);
}

void *kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = kmalloc(total_size);

    if(ptr) {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

void *krealloc(void *ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }

    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    new_size = (new_size + 15) & ~15;

    block_header_t *block = (block_header_t *)((uint8_t *)ptr - HEADER_SIZE);

    if(block->size == new_size) {
        return ptr;
    }

    if(block->size > new_size) {
        if (block->size - new_size >= HEADER_SIZE + 16) {
            block_header_t *remainder = (block_header_t *)((uint8_t *)block + HEADER_SIZE + new_size);
            remainder->size = block->size - new_size - HEADER_SIZE;
            remainder->is_free = true;

            list_add(&remainder->elem, &free_list);

            block->size = new_size;
        }
        return ptr;
    }

    void *new_ptr = kmalloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }

    return new_ptr;
}