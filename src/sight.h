#ifndef SIGHT_H_
#define SIGHT_H_

#include <stdint.h>
#include "heresyrl_def.h"
#include "coord.h"

struct sgt_sight;

struct sgt_sight *sgt_init(void);
void sgt_exit(struct sgt_sight *sight);

bool sgt_calculate_all_light_sources(struct sgt_sight *sight, struct dc_map *map);
bool sgt_calculate_light_source(struct sgt_sight *sight, struct dc_map *map, struct itm_item *item);
bool sgt_calculate_player_sight(struct sgt_sight *sight, struct dc_map *map, struct msr_monster *monster);

int sgt_los_path(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e, coord_t *path_list[]);
bool sgt_has_los(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e);
bool sgt_has_lof(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e);

#endif /*SIGHT_H_*/
