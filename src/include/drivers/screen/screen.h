#pragma once

#include <bootloader/limine.h>
#include <stdint.h>
#include <stddef.h>

void screen_init(struct limine_framebuffer* fb);

void screen_put_pixel(size_t x, size_t y, uint32_t color);

void screen_print_debug(int x, int y, char *text, uint32_t color);