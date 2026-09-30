bits 64
default rel

global thread_trampoline
extern thread_entry_wrapper

section .text

thread_trampoline:
    mov rdi, r15
    mov rsi, r14
    
    call thread_entry_wrapper