#pragma once

#include <bootloader/limine.h>
#include <stdint.h>
#include <stddef.h>

#define DEFAULT_blit32_TEXT_SCALE 2

void screen_init(struct limine_framebuffer* fb);

void screen_put_pixel(size_t x, size_t y, uint32_t color);

void kprint(char *text, uint32_t color, int32_t scale);

void screen_flush(void);

#define kprint_default_scale(text, color) kprint(text, color, DEFAULT_blit32_TEXT_SCALE)