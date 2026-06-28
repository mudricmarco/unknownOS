#pragma once

#include <bootloader/limine.h>

#include <stdint.h>
#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *dest, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *p1, const void *p2, size_t n);