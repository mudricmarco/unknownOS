#pragma once

#include <kernel/thread/thread.h>
#include <klib/list.h>


#define DEFAULT_TIME_SLICE 10 // 10 ticks, which is 1ms

struct list_head *get_ready_queue(void);

thread_t* get_current_thread(void);

void sched_init(void);

