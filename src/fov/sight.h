#ifndef SIGHT_H_
#define SIGHT_H_

#include <stdint.h>
#include "heresyrl_def.h"
#include "coord.h"

struct sgt_sight;

struct sgt_sight *sgt_init(void);
void sgt_exit(struct sgt_sight *sight);

/* loops through the items list, looking for light sources and applies the light on the map. */
bool sgt_calculate_all_light_sources(struct sgt_sight *sight, struct dm_map *map);

bool sgt_calculate_light_source(struct sgt_sight *sight, struct dm_map *map, struct itm_item *item);

/* 
    touches the visible grids within the players far sight and marks them:
      - in_sight

      if lit:
        - visible
        - discovered

      if radius < near_sight:
        visible
        discovered

      if radius < medium_sight:
        discovered
 */
bool sgt_calculate_player_sight(struct sgt_sight *sight, struct dm_map *map, struct msr_monster *monster);

/* returns a traversable point within radius and line of sight from point p */
coord_t sgt_scatter(struct sgt_sight *sight, struct dm_map *map, struct random *r, coord_t *p, int radius);

/* returns an list of grids affected by the 'explosion' at point pos  of size radius */
int sgt_explosion(struct sgt_sight *sight, struct dm_map *map, coord_t *pos, int radius, coord_t *grid_list[]);

/*
   calculates a path from s to e.  path_list must be the address of a 
   coord_t *, which will be malloc'ed by the funtion and must be 
   freed by the caller unless the return value is < 0. 
   continue_path specifies if the path must extend past point e, 
   always up and until an obstruction.  it returns the length of the path.
 */
int sgt_los_path(struct sgt_sight *sight, struct dm_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path, int radius);

/* true if point e is visible from point s.  */
bool sgt_has_los(struct sgt_sight *sight, struct dm_map *map, coord_t *s, coord_t *e, int radius);

/* true if point e is visible and there are no obstructions from point s. */
bool sgt_has_lof(struct sgt_sight *sight, struct dm_map *map, coord_t *s, coord_t *e, int radius);

#endif /*SIGHT_H_*/
