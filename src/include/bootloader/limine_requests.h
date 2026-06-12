#pragma once

#include <bootloader/limine.h>
#include <stdbool.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

bool limine_init_check(void);