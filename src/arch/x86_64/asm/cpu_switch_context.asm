bits 64
default rel

global cpu_switch_context

section .text

; Signature C:
; void cpu_switch_context(uint64_t *old_rsp, uint64_t new_rsp);
;
; Arguments according to the System V ABI:
;   rdi = old_rsp (pointer to uint64_t where to save the old RSP, e.g., &old_thread->rsp)
;   rsi = new_rsp (the new RSP value to load into the CPU, e.g., next_thread->rsp)
cpu_switch_context:
    ; Save callee-saved registers onto the stack of the current thread 
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; Save the current stack pointer (RSP) into the location pointed to by old_rsp (rdi)
    mov [rdi], rsp

    ; Set the stack pointer (RSP) to the new stack pointer (new_rsp) provided in rsi
    mov rsp, rsi

    ; Restore callee-saved registers from the new thread's stack
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ; Return to the new thread's execution context
    ret