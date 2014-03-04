#ifndef SIGHT_H_
#define SIGHT_H_

#include <stdint.h>
#include "heresyrl_def.h"

struct sgt_sight;

struct sgt_sight *sgt_init(void);
void sgt_exit(struct sgt_sight *sight);

bool sgt_calculate_all_light_sources(struct sgt_sight *sight, struct dc_map *map);
bool sgt_calculate_light_source(struct sgt_sight *sight, struct dc_map *map, struct itm_item *item);
bool sgt_calculate_player_sight(struct sgt_sight *sight, struct dc_map *map, struct msr_monster *monster);

#endif /*SIGHT_H_*/
