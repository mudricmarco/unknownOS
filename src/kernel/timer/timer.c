#include <kernel/timer/timer.h>
#include <drivers/screen/screen.h>
#include <klib/list.h>

#ifdef CONFIG_ARCH_X86_64
#include <arch/x86_64/cpu.h>
#endif

// List of registered timer devices
static LIST_HEAD(timer_list);

static timer_device_t* active_timer = NULL;

struct list_head* get_registered_timers(void) {
    return &timer_list;
}

void set_active_timer(timer_device_t *timer) {
    active_timer = timer;
}

timer_device_t* get_active_timer(void) {
    return active_timer;
}

void timer_register(timer_device_t *timer, uint32_t frequency) {
    if (!timer) return;

    timer->frequency = frequency;
    INIT_LIST_HEAD(&timer->list_node);

    if (timer->init) {
        timer->init(frequency);
    }

    list_add_tail(&timer->list_node, &timer_list);

    if (!active_timer) {
        set_active_timer(timer);
    }
}

void sleep_ms(uint32_t ms) {
    if (!active_timer || active_timer->frequency == 0 || !active_timer->get_ticks) {
        return;
    }

    uint64_t start = active_timer->get_ticks();
    
    uint64_t ticks_to_wait = ((uint64_t)ms * active_timer->frequency) / 1000;
    
    if (ticks_to_wait == 0 && ms > 0) {
        ticks_to_wait = 1;
    }

    uint64_t target = start + ticks_to_wait;

    while (active_timer->get_ticks() < target) {
        halt_sti();
    }
}