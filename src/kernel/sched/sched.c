#include <kernel/thread/thread.h>
#include <kernel/sched/sched.h>
#include <kernel/memory/heap.h>
#include <stdint.h>
#include <klib/list.h>
#include <kernel/timer/timer.h>
#include <klib/string.h>

extern void cpu_switch_context(uint64_t *old_rsp, uint64_t new_rsp);;

static LIST_HEAD(ready_queue);

thread_t *current_thread = NULL;

struct list_head *get_ready_queue(void) {
    return &ready_queue;
}

thread_t* get_current_thread(void) {
    return current_thread;
}

void schedule(void) {
    // If there are no threads in the ready queue or only one thread, there's nothing to schedule
    if (list_empty(&ready_queue) || list_is_singular(&ready_queue)) {
        return;
    }

    thread_t *old_thread = current_thread;

    // Selection of the next thread to run for round-robin scheduling
    struct list_head *next_node = old_thread->sched_node.next;
    
    if (next_node == &ready_queue) {
        next_node = next_node->next;
    }

    thread_t *next_thread = list_entry(next_node, thread_t, sched_node);

    // If the next thread is different from the current thread, perform a context switch
    if (old_thread != next_thread) {
        current_thread = next_thread;
        cpu_switch_context(&old_thread->rsp, next_thread->rsp);
    }
}

static void sched_on_tick(void) {
    if (!current_thread) {
        return;
    }

    if (current_thread->time_slice > 0) {
        current_thread->time_slice--;
    }

    if (current_thread->time_slice == 0) {
        current_thread->time_slice = DEFAULT_TIME_SLICE;
        schedule();
    }
}

void sched_init(void) {
    thread_t *main_thread = (thread_t *)kmalloc(sizeof(thread_t));
    
    main_thread->id = new_thread_id();
    main_thread->state = THREAD_STATE_RUNNING;
    main_thread->time_slice = DEFAULT_TIME_SLICE;
    
    main_thread->kernel_stack_base = NULL; 
    main_thread->kernel_stack_size = 0;

    strncpy(main_thread->name, "kernel_main", sizeof(main_thread->name) - 1);
    main_thread->name[sizeof(main_thread->name) - 1] = '\0';

    INIT_LIST_HEAD(&main_thread->sched_node);
    INIT_LIST_HEAD(&main_thread->all_node);

    current_thread = main_thread;

    list_add_tail(&main_thread->sched_node, &ready_queue);

    timer_set_tick_callback(sched_on_tick);
}