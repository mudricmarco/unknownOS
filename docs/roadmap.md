---

kanban-plugin: board

---

## Backlog

- [ ] Ext2 / FAT32 file system implementation
- [ ] VBE / GOP Graphics Mode (GUI rendering)
- [ ] Multi-Core support (SMP)
- [ ] User Mode (Ring 3) & System Calls


## To Do

- [ ] Heap Allocator: Implement basic `kmalloc()` and `kfree()`
- [ ] VMM: Add vmm_unmap_page


## In Progress



## Testing



## Triple Fault/Panic



## Done

- [ ] Bootloader setup using Limine
- [ ] Framebuffer video driver (formatted text, manual pixel drawing, colors)
- [ ] Base `klib` library (math, mem, string, intrusive circular linked list)
- [ ] Serial driver (I/O logging)
- [ ] Kernel Panic handler
- [ ] Physical Memory Manager (PMM)
- [ ] IDT initialization & handling
- [ ] APIC & LAPIC support with delay management
- [ ] PS/2 Keyboard Driver: Handle scancodes via I/O APIC
- [ ] VMM: Implement the `map_page()` function
- [ ] VMM: Initialize PML4, PDPT, PD, and PT page tables
- [ ] VMM: Reload `CR3` with the new kernel page directory




%% kanban:settings
```
{"kanban-plugin":"board","list-collapse":[false,false,false,false,false,false]}
```
%%