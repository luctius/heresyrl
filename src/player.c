#include <ncurses.h>
#include "player.h"
#include "monster.h"
#include "items.h"
#include "fight.h"
#include "inventory.h"

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender) {
    plr->name = name;
    plr->age = 0;
    plr->player = msr_create(race);
    plr->player->icon = '@';
    plr->player->icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL) | A_BOLD;

    plr->player->cur_wounds = 10;
    plr->player->max_wounds = 10;

    plr->weapon_selection = FGHT_WEAPON_SELECT_RIGHT_HAND;
    plr->rof_setting_rhand = WEAPON_ROF_SETTING_SINGLE;
    plr->rof_setting_lhand = WEAPON_ROF_SETTING_SINGLE;
}

void plr_sanity_check(struct pl_player *plr) {
}
