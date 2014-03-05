#ifndef PLAYER_H_
#define PLAYER_H_

#include "heresyrl_def.h"
#include "fight.h"

struct pl_player {
    char *name;
    unsigned long age;
    struct msr_monster *player;
    enum fght_weapon_selection weapon_selection;
    enum fght_weapon_setting weapon_setting_rhand;
    enum fght_weapon_setting weapon_setting_lhand;
};

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender);

#endif /*PLAYER_H_*/
