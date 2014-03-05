#ifndef FIGHT_H
#define FIGHT_H

#include "heresyrl_def.h"
#include "coord.h"

enum fght_weapon_selection {
    FGHT_WEAPON_SELECT_LHAND,
    FGHT_WEAPON_SELECT_RHAND,
    FGHT_WEAPON_SELECT_BHAND,
    FGHT_WEAPON_SELECT_MAX,
};

enum fght_weapon_setting {
    FGHT_WEAPON_SETTING_SINGLE,
    FGHT_WEAPON_SETTING_SEMI,
    FGHT_WEAPON_SETTING_AUTO,
    FGHT_WEAPON_SETTING_MAX,
};

int fght_calc_lof_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);
int fght_shoot(struct msr_monster *monster, struct dc_map *map, enum fght_weapon_selection sel, enum fght_weapon_setting set, coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);

#endif /* FIGHT_H */
