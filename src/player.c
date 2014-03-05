#include <ncurses.h>
#include "player.h"
#include "monster.h"

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender) {
    plr->name = name;
    plr->age = 0;
    plr->player = msr_create(race);
    plr->player->icon = '@';
    plr->player->icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL) | A_BOLD;

    plr->player->cur_wounds = 10;
    plr->player->max_wounds = 10;

    plr->weapon_selection = FGHT_WEAPON_SELECT_RHAND;
    plr->weapon_setting_rhand = FGHT_WEAPON_SETTING_SINGLE;
    plr->weapon_setting_lhand = FGHT_WEAPON_SETTING_SINGLE;
}
