#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"

#include "monster_action.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "monster_static.h"
#include "items.h"
#include "fight.h"
#include "inventory.h"
#include "ui.h"
#include "input.h"
#include "game.h"
#include "event_sys.h"
#include "tiles.h"

static bool ma_check_monster_controller(struct monster_controller *mc) {
    return true;
}

static uint32_t ma_get_energy(void *dataptr) {
    struct msr_monster *monster = dataptr;
    return monster->energy;
}

static bool ma_add_energy(void *dataptr, int energy) {
    struct msr_monster *monster = dataptr;
    monster->energy += energy;
    return true;
}

bool ma_do_guard(struct monster_controller *mc) {
    if (ma_check_monster_controller(mc) == false) return false;
    struct es_event e = {
        .dataptr = mc->monster,
        .controller = mc->controller,
        .ev_do = mc->callback,
        .ev_get = ma_get_energy,
        .ev_set = ma_add_energy,
        .interruptable = false,
        .interrupted = false,
    };
    es_add_event(&e, ENERGY_TURN);
    return true;
}

bool ma_do_move(struct monster_controller *mc, coord_t *newpos) {
    if (ma_check_monster_controller(mc) == false) return false;

    if (msr_move_monster(mc->monster, gbl_game->current_map, newpos) == true) {
        struct es_event e = {
            .dataptr = mc->monster,
            .controller = mc->controller,
            .ev_do = mc->callback,
            .ev_get = ma_get_energy,
            .ev_set = ma_add_energy,
            .interruptable = false,
            .interrupted = false,
        };
        int speed = (msr_calculate_characteristic(mc->monster, MSR_CHAR_AGILITY) / 10) *10;
        es_add_event(&e, ENERGY_TURN -  speed);
        return true;
    }
    return false;
}

bool ma_do_wear(struct monster_controller *mc, struct itm_item *item) {
    if (ma_check_monster_controller(mc) == false) return false;
    return false;
}

bool ma_do_drop(struct monster_controller *mc, struct itm_item *item) {
    if (ma_check_monster_controller(mc) == false) return false;
    return false;
}

bool ma_do_get(struct monster_controller *mc, struct itm_item *item) {
    if (ma_check_monster_controller(mc) == false) return false;
    return false;
}

bool ma_do_throw(struct monster_controller *mc, struct itm_item *item) {
    if (ma_check_monster_controller(mc) == false) return false;
}

