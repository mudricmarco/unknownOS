#pragma once

#include <drivers/screen/screen.h>
#include <drivers/screen/colors.h>

#include <arch/x86_64/registers.h>

void kernel_panic(const char *message);

void kernel_panic_detailed(struct registers* regs, const char* message);