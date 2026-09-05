#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <stddef.h>

static struct {
    struct gdt_entry entries[5];
    struct tss_descriptor tss_desc;
} __attribute__((packed)) gdt;

static struct gdt_ptr gdt_pointer;
static struct tss_entry tss;

static uint8_t double_fault_stack[4096];

static void gdt_set_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt.entries[num].base_low    = (base & 0xFFFF);
    gdt.entries[num].base_middle = (base >> 16) & 0xFF;
    gdt.entries[num].base_high   = (base >> 24) & 0xFF;

    gdt.entries[num].limit_low   = (limit & 0xFFFF);
    gdt.entries[num].granularity = (limit >> 16) & 0x0F;

    gdt.entries[num].granularity |= (gran & 0xF0);
    gdt.entries[num].access      = access;
}

static void gdt_set_tss_descriptor(uint64_t base, uint32_t limit) {
    gdt.tss_desc.limit_low    = (limit & 0xFFFF);
    gdt.tss_desc.base_low     = (base & 0xFFFF);
    gdt.tss_desc.base_middle  = (base >> 16) & 0xFF;
    gdt.tss_desc.access       = 0x89;
    gdt.tss_desc.granularity  = (limit >> 16) & 0x0F;
    gdt.tss_desc.base_high    = (base >> 24) & 0xFF;
    gdt.tss_desc.base_upper32 = (base >> 32) & 0xFFFFFFFF;
    gdt.tss_desc.reserved     = 0;
}

void gdt_init(void) {
    // 0x00: Null Descriptor
    gdt_set_entry(0, 0, 0, 0, 0);

    // 0x08: Kernel Code 64-bit (DPL=0, Long Mode)
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xA0);

    // 0x10: Kernel Data (DPL=0)
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC0);

    // 0x18: User Data (DPL=3)
    gdt_set_entry(3, 0, 0xFFFFF, 0xF2, 0xC0);

    // 0x20: User Code 64-bit (DPL=3, Long Mode)
    gdt_set_entry(4, 0, 0xFFFFF, 0xFA, 0xA0);

    for (size_t i = 0; i < sizeof(tss); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }

    tss.ist1 = (uint64_t)double_fault_stack + sizeof(double_fault_stack);

    gdt_set_tss_descriptor((uint64_t)&tss, sizeof(tss) - 1);

    // Set up the GDT pointer
    gdt_pointer.limit = sizeof(gdt) - 1;
    gdt_pointer.base  = (uint64_t)&gdt;

    // Load the GDT and TSS
    gdt_flush((uint64_t)&gdt_pointer);
}