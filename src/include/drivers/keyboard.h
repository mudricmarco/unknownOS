#pragma once

#include <stdint.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/registers.h>
#endif

void keyboard_init(void);
void keyboard_irq_handler(struct registers* regs);
char keyboard_getchar(void);