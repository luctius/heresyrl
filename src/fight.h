#ifndef FIGHT_H
#define FIGHT_H

#include "heresyrl_def.h"
#include "monster.h"
#include "coord.h"

#define FGHT_RANGED_MODIFIER_OFF_HAND (-20)
#define FGHT_RANGED_MODIFIER_ROF_SEMI (20)
#define FGHT_RANGED_MODIFIER_ROF_AUTO (10)

enum fght_hand {
    FGHT_MAIN_HAND = MSR_WEAPON_SELECT_MAIN_HAND,
    FGHT_OFF_HAND = MSR_WEAPON_SELECT_OFF_HAND,
    FGHT_MAX_HAND,
};

bool fght_do_dmg(struct random *r, struct msr_monster *monster, struct msr_monster *target, int hits, enum fght_hand hand);
int fght_ranged_calc_tohit(struct random *r, struct msr_monster *monster, struct msr_monster *target, enum fght_hand hand);

int fght_shoot(struct random *r, struct msr_monster *monster, struct dc_map *map, coord_t *e);
int fght_calc_lof_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);
const char *fght_weapon_hand_name(enum fght_hand hand);

#endif /* FIGHT_H */
