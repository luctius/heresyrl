#ifndef PLAYER_H_
#define PLAYER_H_

#include "heresyrl_def.h"
#include "fight.h"
#include "coord.h"

void plr_init(struct pl_player *plr, char *name, enum msr_race race, enum msr_gender gender);
struct pf_context *plr_map(struct pl_player *plr, struct dm_map *map);

#endif /*PLAYER_H_*/
