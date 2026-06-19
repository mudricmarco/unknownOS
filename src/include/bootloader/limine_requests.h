#pragma once

#include <bootloader/limine.h>
#include <stdbool.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

extern volatile struct limine_memmap_request memmap_request;

extern volatile struct limine_executable_address_request kernel_address_request;

extern volatile struct limine_hhdm_request hhdm_request;

void limine_init(void);