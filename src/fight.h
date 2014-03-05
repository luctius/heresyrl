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

int fght_calc_lof_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);
int fght_shoot(struct msr_monster *monster, struct dc_map *map, enum fght_weapon_selection sel, enum wpn_rof_setting set1, enum wpn_rof_setting set2, coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);

#endif /* FIGHT_H */
