#include <drivers/screen/screen.h>
#include <stdint.h>
#include <stddef.h>
#include <bootloader/limine.h>

// Blinded configuration of blit32 bitmap font.
#define blit32_ARRAY_ONLY
#define blit32_NO_HELPERS
#define blit_NO_HELPERS
#include <drivers/screen/fonts/blit32.h>

static volatile struct limine_framebuffer* fb = NULL;
static size_t screen_pitch_div_4 = 0;

void screen_init(struct limine_framebuffer* framebuffer) {
    fb = framebuffer;
    screen_pitch_div_4 = fb->pitch / 4;
}

void screen_put_pixel(size_t x, size_t y, uint32_t color) {
    if (fb == NULL) return;    
    ((uint32_t*)fb->address)[y * screen_pitch_div_4 + x] = color;
}

// Basic debug text printing function using the blit32 bitmap font.
void screen_print_debug(int x, int y, char *text, unsigned int color) {
    if (fb == NULL) return;

    blit_props props;
    props.Buffer    = (unsigned int *)fb->address;
    props.BufWidth  = fb->width;
    props.BufHeight = fb->height;
    props.Value     = color;
    props.Scale     = 2;
    props.Wrap      = blit_Clip;

    blit32_TextNExplicit(props.Buffer, props.Value, props.Scale, 
                         props.BufWidth, props.BufHeight, props.Wrap, 
                         x, y, -1, text);
}