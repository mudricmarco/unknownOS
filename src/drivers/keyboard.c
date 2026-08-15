#include <stdint.h>
#include <stdbool.h>
#include <klib/list.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#include <arch/x86_64/registers.h>
#include <arch/x86_64/drivers/ioapic.h>
#include <arch/x86_64/lapic.h>
#include <arch/x86_64/io.h>
#include <arch/x86_64/drivers/ps2.h>
#endif

#define KBD_BUFFER_SIZE 128

struct kbd_event {
    char c;
    struct list_head list;
};

static struct kbd_event kbd_pool[KBD_BUFFER_SIZE];

static LIST_HEAD(kbd_free_list);
static LIST_HEAD(kbd_pending_list);

static bool shift_pressed = false;
static bool scancode_down[256] = { false };

static const char scancode_ascii_nomod[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',    0, ' '
};

static const char scancode_ascii_shift[] = {
    0,  27, '!', '"', '#', '$', '%', '&', '/', '(', ')', '=', '?', '^', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', '~',
    0, '>',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_',   0,
  '*',    0, ' '
};

static void keyboard_process_scancode(uint8_t scancode) {
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return;
    }

    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
        return;
    }

    if (scancode & 0x80) {
        scancode_down[scancode & 0x7F] = false;
        return;
    }

    if (scancode_down[scancode]) {
        return;
    }

    scancode_down[scancode] = true;
    
    if (scancode < sizeof(scancode_ascii_nomod)) {
        char c = shift_pressed ? scancode_ascii_shift[scancode] : scancode_ascii_nomod[scancode];
        if (c != 0 && !list_empty(&kbd_free_list)) {
            struct list_head *node = kbd_free_list.next;
            list_del(node);

            struct kbd_event *evt = list_entry(node, struct kbd_event, list);
            evt->c = c;

            list_add_tail(&evt->list, &kbd_pending_list);
        }
    }
}

void keyboard_init(void) {
    ioapic_init_keyboard((uint8_t)(lapic_read(LAPIC_ID) >> 24));
    ps2_keyboard_enable();
    INIT_LIST_HEAD(&kbd_free_list);
    INIT_LIST_HEAD(&kbd_pending_list);

    for (int i = 0; i < KBD_BUFFER_SIZE; i++) {
        list_add_tail(&kbd_pool[i].list, &kbd_free_list);
    }
}

void keyboard_irq_handler(struct registers* regs) {
    (void)regs;

    uint8_t scancode = inb(0x60);

    keyboard_process_scancode(scancode);

    lapic_eoi();
}

char keyboard_getchar(void) {
    for (;;) {
        disable_interrupts();

        if (!list_empty(&kbd_pending_list)) {
            struct list_head *node = kbd_pending_list.next;
            list_del(node);

            struct kbd_event *evt = list_entry(node, struct kbd_event, list);
            char c = evt->c;

            list_add_tail(&evt->list, &kbd_free_list);

            enable_interrupts();
            return c;
        }

        enable_interrupts();
        __asm__ volatile("pause");
    }
}