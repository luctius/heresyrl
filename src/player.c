#include <ncurses.h>
#include "player.h"
#include "monster.h"
#include "monster_static.h"
#include "items.h"
#include "fight.h"
#include "inventory.h"

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender) {
    if (plr->player == NULL) {
        plr->name = name;
        plr->player = msr_create(MSR_ID_BASIC_FERAL);

        plr->weapon_selection = FGHT_WEAPON_SELECT_RIGHT_HAND;
        plr->rof_setting_rhand = WEAPON_ROF_SETTING_SINGLE;
        plr->rof_setting_lhand = WEAPON_ROF_SETTING_SINGLE;
    }

    plr->player->icon = '@';
    plr->player->icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL) | A_BOLD;
}

void plr_sanity_check(struct pl_player *plr) {
}
