#include <stdio.h>
#include <string.h>
#include <sys/queue.h>

#include "ground_effects.h"
#include "dungeon/tiles.h"
#include "turn_tick.h"
#include "logging.h"
#include "random.h"

struct ground_effect_list_entry {
    struct ground_effect ground_effect;
    TAILQ_ENTRY(ground_effect_list_entry) entries;
};
static TAILQ_HEAD(ground_effects_list, ground_effect_list_entry) ground_effects_list_head;
static bool ground_effects_list_initialised = false;

#include "ground_effects_static_def.h"

/* Garanteed to be random, rolled it myself ;)  */
#define GROUND_EFFECT_PRE_CHECK         (72321)
#define GROUND_EFFECT_POST_CHECK        (4557)
#define GROUND_EFFECT_LIST_PRE_CHECK    (1324)
#define GROUND_EFFECT_LIST_POST_CHECK   (7892)

void ge_init(void) {
    for (unsigned int i = 0; i < GEID_MAX; i++) {
        const struct ground_effect *ground_effect = &static_ground_effect_list[i];
        if (ground_effect->tid != i) {
            fprintf(stderr, "Ground Effects list integrity check failed! [%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }

    if (ground_effects_list_initialised == false) {
        TAILQ_INIT(&ground_effects_list_head);
        ground_effects_list_initialised = true;
    }
}

void ge_exit(void) {
    struct ground_effect_list_entry *e = NULL;
    while (ground_effects_list_head.tqh_first != NULL) {
        e = ground_effects_list_head.tqh_first;
        TAILQ_REMOVE(&ground_effects_list_head, ground_effects_list_head.tqh_first, entries);
        free(e);
    }
    ground_effects_list_initialised = false;
}

struct ground_effect *gelst_ground_effect_by_uid(uint32_t ground_effect_uid) {
    if (ground_effects_list_initialised == false) return false;
    struct ground_effect_list_entry *sele = ground_effects_list_head.tqh_first;

    while (sele != NULL) {
        if (ground_effect_uid == sele->ground_effect.uid) return &sele->ground_effect;
        sele = sele->entries.tqe_next;
    }
    return NULL;
}

static uint32_t gelst_next_id(void) {
    if (ground_effects_list_initialised == false) return false;
    struct ground_effect_list_entry *sele = ground_effects_list_head.tqh_first;
    uint32_t uid = 1;

    while (sele != NULL) {
        if (uid <= sele->ground_effect.uid) uid = sele->ground_effect.uid+1;
        sele = sele->entries.tqe_next;
    }
    return uid;
}

struct ground_effect *gelst_get_next(struct ground_effect *prev) {
    if (prev == NULL) {
        if (ground_effects_list_head.tqh_first != NULL) return &ground_effects_list_head.tqh_first->ground_effect;
        return NULL;
    }
    struct ground_effect_list_entry *gele = container_of(prev, struct ground_effect_list_entry, ground_effect);
    if (gele == NULL) return NULL;
    return &gele->entries.tqe_next->ground_effect;
}

struct ground_effect *ge_create(uint32_t tid, struct dm_map_entity *me) {
    if (ground_effects_list_initialised == false) return NULL;
    if (me->effect != NULL) return NULL;
    if (tid == GEID_NONE) return NULL;
    if (tid >= GEID_MAX) return NULL;
    if (tid >= (int) ARRAY_SZ(static_ground_effect_list) ) return NULL;
    if (TILE_HAS_ATTRIBUTE(me->tile, TILE_ATTR_TRAVERSABLE) == false) return NULL;

    const struct ground_effect *ge_template = &static_ground_effect_list[tid];

    struct ground_effect_list_entry *gele = malloc(sizeof(struct ground_effect_list_entry) );
    if (gele == NULL) return NULL;

    memcpy(&gele->ground_effect, ge_template, sizeof(struct ground_effect) );
    TAILQ_INSERT_TAIL(&ground_effects_list_head, gele, entries);
    struct ground_effect *ge = &gele->ground_effect;
    ge->ground_effect_pre = GROUND_EFFECT_PRE_CHECK;
    ge->ground_effect_post = GROUND_EFFECT_POST_CHECK;
    ge->uid = gelst_next_id();

    int range = (ge->max_energy - ge->min_energy);
    ge->current_energy = ge->min_energy;
    if (range > 0) ge->current_energy += (random_int32(gbl_game->random) % range);
    if (ge->current_energy == 0) ge->current_energy = 1;
    //lg_debug("Creating ge: %p(%s) duration: %d, max: %d", ge, ge->sd_name, ge->current_energy, ge->max_energy);
    ge->max_energy = ge->current_energy;

    me->effect = ge;
    ge->me = me;

    return ge;
}

bool ge_destroy(struct dm_map_entity *me) {
    if (ground_effects_list_initialised == false) return false;
    struct ground_effect_list_entry *gele = ground_effects_list_head.tqh_first;

    while (gele != NULL) {
        struct ground_effect *ge = &gele->ground_effect;
        if (me->effect == ge) {
            //lg_debug("Destroying ge: %p(%s) duration: %d, max: %d", ge, ge->sd_name, ge->current_energy, ge->max_energy);
            TAILQ_REMOVE(&ground_effects_list_head, gele, entries);
            me->effect = NULL;
            free(gele);
            return true;
        }
        gele = gele->entries.tqe_next;
    }

    return false;
}

void ge_process(struct dm_map *map) {
    if (ground_effects_list_initialised == false) return;
    struct ground_effect_list_entry *sele = ground_effects_list_head.tqh_first;

    while (sele != NULL) {
        struct ground_effect *ge = &sele->ground_effect;
        ge->current_energy -= MIN(TT_ENERGY_TICK, ge->current_energy);

        /* TODO: check if any monster here is affected by the se_id */
        if (ge->me->monster != NULL) {
            if (ge->flags & GR_EFFECTS_REMOVE_ON_EXIT > 0) {
                se_remove_effects_by_tid(ge->me->monster, ge->tid);
            }
        }
        if (ge->current_energy <= 0) {
            ge_destroy(ge->me);
        }

        sele = sele->entries.tqe_next;
    }
}

