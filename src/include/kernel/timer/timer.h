#pragma once

#include <stdint.h>
#include <klib/list.h>

typedef struct timer_device_t {
    const char* name;
    void (*init)(uint32_t frequency);
    uint64_t (*get_ticks)(void);
    uint32_t frequency;
    struct list_head list_node;
} timer_device_t;

void timer_register(timer_device_t *timer, uint32_t frequency);
struct list_head* get_registered_timers(void);
void set_active_timer(timer_device_t *timer);
timer_device_t* get_active_timer(void);

void sleep_ms(uint32_t ms);