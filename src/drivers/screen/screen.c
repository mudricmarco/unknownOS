#include "kernel/panic.h"
#include <drivers/screen/screen.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <klib/mem.h>
#include <klib/math.h>
#include <klib/string.h>
#include <bootloader/limine.h>
#include <bootloader/limine_requests.h>
#include <kernel/panic.h>
#include <kernel/memory/pmm.h>

// Blinded configuration of blit32 bitmap font
#define blit32_ARRAY_ONLY
#define blit32_NO_HELPERS
#define blit32_NO_INLINE
#include <drivers/screen/fonts/blit32.h>

static struct limine_framebuffer* fb = NULL;
static size_t screen_pitch_div_4 = 0;

static uint32_t* backbuffer = NULL;

#define DEFAULT_CURSOR_X 10
#define DEFAULT_CURSOR_Y 10

static int32_t cursor_x = DEFAULT_CURSOR_X;
static int32_t cursor_y = DEFAULT_CURSOR_Y;

#define blit32_FONT_WIDTH blit32_WIDTH
#define blit32_FONT_HEIGHT blit32_HEIGHT

#define TAB_SIZE 4

static bool auto_flush_enabled = true;

void set_auto_flush(bool enable) {
    auto_flush_enabled = enable;
}

bool get_auto_flush() {
    return auto_flush_enabled;
}


// Helper function to reset cursor position to the top-left corner of the screen
void reset_cursor(void) {
    cursor_x = DEFAULT_CURSOR_X;
    cursor_y = DEFAULT_CURSOR_Y;
}

// Method to flush the backbuffer to the actual framebuffer in VRAM
void screen_flush(void) {
    if (fb == NULL) return;

    for (size_t y = 0; y < fb->height; y++) {
        uint32_t* dest = ((uint32_t*)fb->address) + (y * screen_pitch_div_4);
        uint32_t* src  = backbuffer + (y * screen_pitch_div_4);
    
        memcpy((void*)dest, (void*)src, fb->width * 4);
    }
}

// Method to set a pixel in the backbuffer
void screen_put_pixel(size_t x, size_t y, uint32_t color, bool direct_vram) {
    if (fb == NULL || x >= fb->width || y >= fb->height) return;

    if (direct_vram) {
        uint32_t* pixel_addr = ((uint32_t*)fb->address) + (y * screen_pitch_div_4) + x;
        *pixel_addr = color;
    } else {
        backbuffer[y * screen_pitch_div_4 + x] = color;
    }
}

static void check_newline_and_scroll(blit_props* props, int32_t cell_height, bool direct_vram) {
    cursor_x = DEFAULT_CURSOR_X;
    cursor_y += cell_height;

    if (cursor_y + cell_height > props->BufHeight) {
        size_t stride = screen_pitch_div_4;
        int32_t line_size = cell_height * stride;
        
        if(direct_vram) {
            uint32_t* vram = (uint32_t*)fb->address;
            for (int32_t y = 0; y < props->BufHeight - cell_height; y++) {
                uint32_t* dest = vram + (y * stride);
                uint32_t* src  = vram + ((y + cell_height) * stride);
                memcpy((void*)dest, (void*)src, props->BufWidth * 4);
            }
            memset((void*)(vram + (props->BufHeight - cell_height) * stride), 
                   0, 
                   cell_height * stride * 4);
        } else {
            memmove((void*)backbuffer, 
                    (void*)(backbuffer + line_size), 
                    (props->BufHeight - cell_height) * stride * 4);
                
            memset((void*)(backbuffer + (props->BufHeight - cell_height) * stride), 
                0, 
                cell_height * stride * 4);
        }
        
        cursor_y -= cell_height;
    }
}

// Basic text printing function using the blit32 bitmap font.
void kprint(const char *text, uint32_t color, int32_t scale, bool direct_vram) {
    if (fb == NULL || text == NULL) return;

    blit_props props;
    props.Buffer    = direct_vram ? (uint32_t*)fb->address : backbuffer;
    props.BufWidth  = (int32_t)screen_pitch_div_4;
    props.BufHeight = (int32_t)fb->height;
    props.Value     = color;
    props.Scale     = scale;
    props.Wrap      = blit_Clip;

    int32_t cell_width = (blit32_FONT_WIDTH + 2) * scale;
    int32_t cell_height = (blit32_FONT_HEIGHT + 2) * scale;

    for (size_t i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            check_newline_and_scroll(&props, cell_height, direct_vram);
            continue; 
        }

        if (text[i] == '\r') {
            cursor_x = DEFAULT_CURSOR_X;
            continue; 
        }

        if (text[i] == '\b') {
            cursor_x -= cell_width;
            if (cursor_x < DEFAULT_CURSOR_X) {
                cursor_x = DEFAULT_CURSOR_X;
            }
            continue; 
        }

        if (text[i] == '\t') {
            int32_t tab_width = cell_width * TAB_SIZE;
            if (cursor_x + tab_width > (int32_t)fb->width) {
                check_newline_and_scroll(&props, cell_height, direct_vram);
            } else {
                for (int t = 0; t < TAB_SIZE; t++) {
                    if (cursor_x + cell_width <= (int32_t)fb->width) {
                        blit32_TextNExplicit(props.Buffer, props.Value, props.Scale, 
                                             props.BufWidth, props.BufHeight, props.Wrap, 
                                             cursor_x, cursor_y, -1, " ");
                        cursor_x += cell_width;
                    }
                }
            }
            continue;
        }

        if (cursor_x + cell_width > (int32_t)fb->width) {
            check_newline_and_scroll(&props, cell_height, direct_vram);
        }

        char single_char_str[2] = { text[i], '\0' };

        blit32_TextNExplicit(props.Buffer, props.Value, props.Scale, 
                             props.BufWidth, props.BufHeight, props.Wrap, 
                             cursor_x, cursor_y, -1, single_char_str);

        cursor_x += cell_width;
        
    }
}

// Formatted printing function that supports color changes, integers, and strings.
void kprintf(uint32_t default_color, int32_t scale, bool direct_vram, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    uint32_t current_color = default_color;
    
    char chunk_buffer[256];
    size_t chunk_idx = 0;

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            chunk_buffer[chunk_idx++] = fmt[i];
            
            if (chunk_idx == sizeof(chunk_buffer) - 1) {
                chunk_buffer[chunk_idx] = '\0';
                kprint(chunk_buffer, current_color, scale, direct_vram);
                chunk_idx = 0;
            }
            continue;
        }

        if (chunk_idx > 0) {
            chunk_buffer[chunk_idx] = '\0';
            kprint(chunk_buffer, current_color, scale, direct_vram);
            chunk_idx = 0;
        }

        i++;

        switch (fmt[i]) {
            case 'C': {
                current_color = va_arg(args, uint32_t);
                break;
            }
            case 'd': {
                int64_t num = va_arg(args, int64_t); 
                char str_buffer[64]; 
                int_to_string(num, str_buffer); 
                kprint(str_buffer, current_color, scale, direct_vram);
                break;
            }
            case 's': {
                char* str = va_arg(args, char*);
                if (str == NULL) str = "(null)"; 
                kprint(str, current_color, scale, direct_vram);
                break;
            }
            case 'x': {
                uint64_t num = va_arg(args, uint64_t); 
                char hex_buffer[32]; 
                int_to_hex_string(num, hex_buffer); 
                kprint(hex_buffer, current_color, scale, direct_vram);
                break;
            }
            case '%': {
                kprint("%", current_color, scale, direct_vram);
                break;
            }
            default:
                break;
        }
    }
    
    if (chunk_idx > 0) {
        chunk_buffer[chunk_idx] = '\0';
        kprint(chunk_buffer, current_color, scale, direct_vram);
    }
    
    if (!direct_vram && auto_flush_enabled) {
        screen_flush();
    }

    va_end(args);
}

struct limine_framebuffer* screen_get_fb(void) {
    return fb;
}

bool is_screen_initialized(void) {
    return fb != NULL;
}

void screen_set_cursor(int32_t x, int32_t y) {
    cursor_x = x;
    cursor_y = y;
}

int32_t screen_get_cursor_y(void) {
    return cursor_y;
}

int32_t screen_get_cursor_x(void) {
    return cursor_x;
}

void screen_init() {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        //? This message will only be printed to the serial console, as we don't have a framebuffer to display it on the screen.
        kernel_panic("Failed to initialize screen: No valid framebuffer available.");
    }

    fb = framebuffer_request.response->framebuffers[0];

    // -- Check if the framebuffer meets the kernel's requirements --
    if (fb->bpp != 32) {
        kernel_panic("Unsupported BPP. Kernel requires exactly 32 bits per pixel.");
    }
    if (fb->memory_model != LIMINE_FRAMEBUFFER_RGB) {
        kernel_panic("Unsupported memory model. Kernel requires RGB/RGBA framebuffer.");
    }
    if (fb->red_mask_size != 8 || fb->green_mask_size != 8 || fb->blue_mask_size != 8) {
        kernel_panic("Unexpected color mask size.");
    }

    screen_pitch_div_4 = fb->pitch / 4;
    
    uint64_t total_bytes = fb->pitch * fb->height;
    uint64_t pages_needed = (total_bytes + 4095) / 4096;
    
    void* phys_buffer = pmm_alloc_pages(pages_needed);

    if (phys_buffer == NULL) {
        kernel_panic("Failed to initialize screen: Unable to allocate backbuffer for screen");
    }

    backbuffer = (uint32_t*)((uint64_t)phys_buffer + get_hhdm_offset());
    
    memset((void*)backbuffer, 0, total_bytes);
}

void screen_clear(uint32_t color, bool direct_vram) {
    if (fb == NULL) return;

    uint32_t* target = direct_vram ? (uint32_t*)fb->address : backbuffer;

    if (color == 0) {
        memset((void*)target, 0, fb->pitch * fb->height);
    } 
    else {
        for (size_t y = 0; y < fb->height; y++) {
            uint32_t* line = (uint32_t*)((uint8_t*)target + (y * fb->pitch));
            for (size_t x = 0; x < fb->width; x++) {
                line[x] = color;
            }
        }
    }

    reset_cursor();

    if (!direct_vram && auto_flush_enabled) {
        screen_flush();
    }
}