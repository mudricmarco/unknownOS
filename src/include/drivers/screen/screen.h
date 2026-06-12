#pragma once

#include <bootloader/limine.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DEFAULT_blit32_TEXT_SCALE 2

struct limine_framebuffer *screen_get_fb(void);

void screen_init(struct limine_framebuffer* fb);

void screen_put_pixel(size_t x, size_t y, uint32_t color, bool direct_vram);

void screen_clear(uint32_t color, bool direct_vram);

void kprint(const char *text, uint32_t color, int32_t scale, bool direct_vram);

void screen_flush(void);

#define kprint_default_scale(text, color, direct_vram) kprint(text, color, DEFAULT_blit32_TEXT_SCALE, direct_vram)