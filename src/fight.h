#ifndef FIGHT_H
#define FIGHT_H

#include "heresyrl_def.h"
#include "coord.h"

enum fght_weapon_selection {
    FGHT_WEAPON_SELECT_LEFT_HAND,
    FGHT_WEAPON_SELECT_RIGHT_HAND,
    FGHT_WEAPON_SELECT_DUAL_HAND,
    FGHT_WEAPON_SELECT_BOTH_HAND,
    FGHT_WEAPON_SELECT_MAX,
};

int fght_calc_lof_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);
int fght_shoot(struct msr_monster *monster, struct dc_map *map, enum fght_weapon_selection sel, enum wpn_rof_setting set1, enum wpn_rof_setting set2, coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);
bool fght_weapons_check(struct msr_monster *monster, enum fght_weapon_selection sel);
bool fght_ranged_weapons_check(struct msr_monster *monster, enum fght_weapon_selection sel);

#endif /* FIGHT_H */
