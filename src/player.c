#include <ncurses.h>
#include "player.h"
#include "monster.h"
#include "items.h"
#include "inventory.h"

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender) {
    plr->name = name;
    plr->age = 0;
    plr->player = msr_create(race);
    plr->player->icon = '@';
    plr->player->icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL) | A_BOLD;

    plr->player->cur_wounds = 10;
    plr->player->max_wounds = 10;

    plr->weapon_selection = FGHT_WEAPON_SELECT_RHAND;
    plr->rof_setting_rhand = WPN_ROF_SETTING_SINGLE;
    plr->rof_setting_lhand = WPN_ROF_SETTING_SINGLE;
}

void plr_sanity_check(struct pl_player *plr) {
}

bool plr_ranged_weapons_check(struct pl_player *plr) {
    if (plr->weapon_selection == FGHT_WEAPON_SELECT_BHAND) {
        if (inv_loc_empty(plr->player->inventory, INV_LOC_BOTH_WIELD) ) return false;
        if (wpn_is_type(inv_get_item_from_location(plr->player->inventory, INV_LOC_BOTH_WIELD), WEAPON_TYPE_RANGED) == false) return false;
    }
    if (plr->weapon_selection == FGHT_WEAPON_SELECT_RHAND) {
        if (inv_loc_empty(plr->player->inventory, INV_LOC_RIGHT_WIELD) ) return false;
        if (wpn_is_type(inv_get_item_from_location(plr->player->inventory, INV_LOC_RIGHT_WIELD), WEAPON_TYPE_RANGED) == false) return false;
    }
    if (plr->weapon_selection == FGHT_WEAPON_SELECT_LHAND) {
        if (inv_loc_empty(plr->player->inventory, INV_LOC_LEFT_WIELD) ) return false;
        if (wpn_is_type(inv_get_item_from_location(plr->player->inventory, INV_LOC_LEFT_WIELD), WEAPON_TYPE_RANGED) == false) return false;
    }

}
