#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <klib/list.h>

typedef struct block_header {
    size_t size;
    bool is_free;
    struct list_head elem;
} block_header_t;

#define HEADER_SIZE ((sizeof(block_header_t) + 15) & ~15)

void heap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t num, size_t size);
void *krealloc(void *ptr, size_t new_size);