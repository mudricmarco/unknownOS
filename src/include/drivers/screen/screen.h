#pragma once

#include <bootloader/limine.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#define DEFAULT_blit32_TEXT_SCALE 2

void set_auto_flush(bool enable);
bool get_auto_flush(void);

void reset_cursor(void);

void screen_flush(void);

void screen_put_pixel(size_t x, size_t y, uint32_t color, bool direct_vram);

void kprint(const char *text, uint32_t color, int32_t scale, bool direct_vram);

struct limine_framebuffer* screen_get_fb(void);

bool is_screen_initialized(void);

void screen_set_cursor(int32_t x, int32_t y);

int32_t screen_get_cursor_y(void);

int32_t screen_get_cursor_x(void);

void screen_init();

void screen_clear(uint32_t color, bool direct_vram);

void kprintf(uint32_t color, int32_t scale, bool direct_vram, const char* fmt, ...);

#define kprint_default_scale(text, color, direct_vram) kprint(text, color, DEFAULT_blit32_TEXT_SCALE, direct_vram)
#define kprintf_default(fmt, ...) kprintf(COLOR_WHITE, DEFAULT_blit32_TEXT_SCALE, false, fmt, ##__VA_ARGS__)
#define kprintf_default_scale(color, direct_vram, fmt, ...) kprintf(color, DEFAULT_blit32_TEXT_SCALE, direct_vram, fmt, ##__VA_ARGS__)
#define kprintf_default_scale_no_color(direct_vram, fmt, ...) kprintf(COLOR_WHITE, DEFAULT_blit32_TEXT_SCALE, direct_vram, fmt, ##__VA_ARGS__)