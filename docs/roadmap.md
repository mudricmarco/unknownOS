---

kanban-plugin: board

---

## Backlog

- [ ] Kernel Threads & Task Control Block (PCB) structure
- [ ] Context Switching & Timer-based Preemptive Scheduler
- [ ] User Mode (Ring 3) transition (`sysret`/`syscall`)
- [ ] System Call dispatcher and core syscalls (`write`, `exit`, etc.)
- [ ] PCI Bus Enumeration driver
- [ ] AHCI / SATA storage controller driver
- [ ] Virtual File System (VFS) layer abstraction
- [ ] Ext2 / FAT32 file system implementation
- [ ] Dynamic ELF Binary Loader (execute binaries from disk)
- [ ] Inter-Process Communication (IPC) / Pipes
- [ ] Multi-Core support (SMP) & AP initialization
- [ ] VBE / GOP Graphics Mode (GUI rendering) & mouse support


## To Do



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
- [ ] VMM: Add vmm_unmap_page
- [ ] Heap Allocator: Implement basic `kmalloc()` and `kfree()`




%% kanban:settings
```
{"kanban-plugin":"board","list-collapse":[false,false,false,false,false,false]}
```
%%