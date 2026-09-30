#include <stdint.h>
#include <stddef.h>
#include <klib/list.h>
#include <kernel/memory/heap.h>
#include <kernel/thread/thread.h>
#include <klib/string.h>
#include <arch/x86_64/cpu.h>

// TODO: Should be defined in the architecture-specific code
extern void thread_trampoline(void);

thread_id_t new_thread_id(void) {
    static thread_id_t next_id = 1;
    return next_id++;
}

void thread_entry_wrapper(void (*entry)(void *), void *arg) {
    if (entry != NULL) {
        entry(arg);
    }

    // TODO: Change it to use a thread exit function
    hcf();
}

thread_t *thread_create(const char *name, void (*entry)(void *), void *arg) {
    if(name == NULL || entry == NULL) {
        return NULL;
    }

    thread_t *new_thread = (thread_t *)kmalloc(sizeof(thread_t));

    if(new_thread == NULL) {
        return NULL;
    }

    new_thread->kernel_stack_base = kmalloc(MAX_STACK_SIZE);
    if(new_thread->kernel_stack_base == NULL) {
        kfree(new_thread);
        return NULL;
    }

    uint64_t *stack = (uint64_t *)((uint8_t *)new_thread->kernel_stack_base + MAX_STACK_SIZE);

    *(--stack) = (uint64_t)thread_trampoline; // Return address for the context switch to jump to
    *(--stack) = 0; // RBP
    *(--stack) = 0; // RBX
    *(--stack) = 0; // R12
    *(--stack) = 0; // R13
    *(--stack) = (uint64_t)arg; // R14
    *(--stack) = (uint64_t)entry; // R15

    // Initialize the thread structure
    new_thread->kernel_stack_size = MAX_STACK_SIZE;
    new_thread->rsp = (uint64_t)stack;
    new_thread->id = new_thread_id();
    new_thread->state = THREAD_STATE_NEW;

    strncpy(new_thread->name, name, sizeof(new_thread->name) - 1);
    new_thread->name[sizeof(new_thread->name) - 1] = '\0';

    INIT_LIST_HEAD(&new_thread->sched_node);
    INIT_LIST_HEAD(&new_thread->all_node);

    return new_thread;
}

void thread_destroy(thread_t *thread) {
    if(thread == NULL) {
        return;
    }

    if(thread->kernel_stack_base != NULL) {
        kfree(thread->kernel_stack_base);
    }

    kfree(thread);
}