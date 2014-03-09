#ifndef EVENT_SYS_H
#define EVENT_SYS_H

#include <limits.h>

#include "heresyrl_def.h"

struct es_event_sys;
struct es_event;

typedef uint32_t (*event_get_energy_t)(void *dataptr);
typedef bool (*event_add_energy_t)(void *dataptr, int energy);
typedef bool (*event_callback_t)(void *dataptr, void *controller, bool interrupted);

struct es_event {
    void *dataptr; 
    void *controller;
    event_callback_t ev_do;
    event_get_energy_t ev_get;
    event_add_energy_t ev_set;
    bool interruptable;
    bool interrupted;
};

struct es_event_sys *es_init(void);
void es_exit(struct es_event_sys *es_ctx);

bool es_process(struct es_event_sys *es_ctx);
bool es_add_event(struct es_event_sys *es_ctx, struct es_event *event, int cost);
bool es_interrupt_event(struct es_event_sys *es_ctx, void *dataptr_to_interrupt);


#endif /* EVENT_SYS_H */
