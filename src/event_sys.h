#ifndef EVENT_SYS_H
#define EVENT_SYS_H

#include <limits.h>

#include "heresyrl_def.h"

#define ENERGY_MAX (UINT32_MAX - 1000)
#define ENERGY_TICK (10)

struct es_event_sys;
struct es_event;

struct es_event {
    void *dataptr; 
    void *controller;
    event_callback_t ev_do;
    event_get_energy_t ev_get;
    event_add_energy_t ev_set;
    bool interruptable;
    bool interrupted;
};

void es_init(void);
void es_exit(void);

bool es_process(void);
bool es_add_event(struct es_event *event, int cost);
bool es_interrupt_event(void *dataptr_to_interrupt);


#endif /* EVENT_SYS_H */
