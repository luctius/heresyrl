#ifndef MONSTER_TURN_H
#define MONSTER_TURN_H

#include <limits.h>

#include "heresyrl_def.h"
#include "monster.h"
#include "coord.h"

enum MSR_ACTION_COSTS {
    MSR_ACTION_MOVE   = MSR_ENERGY_TURN, /* minus agility bonus * 10 */
    MSR_ACTION_IDLE   = MSR_ENERGY_TURN / 2,
    MSR_ACTION_WEAR   = MSR_ENERGY_TURN, /* times use_delay */ 
    MSR_ACTION_REMOVE = MSR_ENERGY_TURN, /* times use_delay */
    MSR_ACTION_PICKUP = MSR_ENERGY_TURN, /* times nr items */
    MSR_ACTION_DROP   = MSR_ENERGY_TURN, /* times nr items */
    MSR_ACTION_USE    = MSR_ENERGY_TURN, /* times use_delay */
    MSR_ACTION_RELOAD = MSR_ENERGY_TURN, /* times use_delay */
    MSR_ACTION_UNLOAD = MSR_ENERGY_TURN, /* times use_delay */

    MSR_ACTION_GUARD  = MSR_ENERGY_TURN,
    MSR_ACTION_THROW  = MSR_ENERGY_TURN,
    MSR_ACTION_MELEE  = MSR_ENERGY_TURN,
    MSR_ACTION_FIRE   = MSR_ENERGY_TURN,
    MSR_ACTION_SINGLE_SHOT  = MSR_ENERGY_TURN / 2,
    MSR_ACTION_SINGLE_MELEE = MSR_ENERGY_TURN / 2,
};

void ma_init(void);
void ma_exit(void);

bool ma_process(void);
bool ma_interrupt_event(uint32_t monster_uid);

bool ma_do_move(struct msr_monster *monster, coord_t *pos);
int ma_do_move_cost(struct msr_monster *monster, coord_t *pos);
bool ma_do_idle(struct msr_monster *monster);
int ma_do_idle_cost(struct msr_monster *monster);

bool ma_do_wear(struct msr_monster *monster, struct itm_item *item);
bool ma_do_remove(struct msr_monster *monster, struct itm_item *item);
bool ma_do_use(struct msr_monster *monster, struct itm_item *item);
bool ma_do_drop(struct msr_monster *monster, struct itm_item *items[], int nr_items);
bool ma_do_pickup(struct msr_monster *monster, struct itm_item *items[], int nr_items);

bool ma_do_guard(struct msr_monster *monster);
bool ma_do_throw(struct msr_monster *monster, coord_t *pos);
bool ma_do_melee(struct msr_monster *monster, coord_t *pos);
bool ma_do_fire(struct msr_monster *monster, coord_t *pos);

/*  ammo_item can be NULL, it will then search for similar ammo.  */
bool ma_do_reload_carried(struct msr_monster *monster, struct itm_item *ammo_item);
/*  ammo_item can be NULL, it will then search for similar ammo.  */
bool ma_do_reload_weapon(struct msr_monster *monster, struct itm_item *weapon_item, struct itm_item *ammo_item);

/*  weapon_item can be NULL, it will then unload either the first loaded weapon 
    on the ground, or it will unload all carried weapons. */
bool ma_do_unload(struct msr_monster *monster, struct itm_item *weapon_item);

#endif /* MONSTER_TURN_H */
