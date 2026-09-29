#pragma once

#include <stdint.h>
#include <stddef.h>
#include <klib/list.h>

#define MAX_STACK_SIZE (8*1024) // 8 KiB

typedef uint64_t thread_id_t;

typedef enum thread_state {
    THREAD_STATE_NEW,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_TIMED_WAIT,
    THREAD_STATE_TERMINATED
} thread_state_t;

typedef struct thread {
    uint64_t rsp;
    thread_id_t id;
    char name[32];
    thread_state_t state;
    void *kernel_stack_base;
    size_t kernel_stack_size;
    struct list_head sched_node;
    struct list_head all_node;
} thread_t;

thread_t *thread_create(const char *name, void (*entry)(void *));
void thread_destroy(thread_t *thread);