#ifndef PLAYER_H_
#define PLAYER_H_

#include "heresyrl_def.h"
#include "fight.h"
#include "items.h"

struct pl_player {
    char *name;
    struct msr_monster *player;
};

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender);

#endif /*PLAYER_H_*/
