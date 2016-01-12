/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SIGHT_H_
#define SIGHT_H_

#include <stdint.h>
#include "heresyrl_def.h"
#include "coord.h"

/* loops through the items list, looking for light sources and applies the light on the map. */
bool sgt_calculate_all_light_sources(struct dm_map *map);

bool sgt_calculate_light_source(struct dm_map *map, struct itm_item *item);

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
bool sgt_calculate_player_sight(struct dm_map *map, struct msr_monster *monster);

/* returns a traversable point within radius and line of sight from point p */
coord_t sgt_scatter(struct dm_map *map, struct random *r, coord_t *p, int radius);

/* returns an list of grids affected by the 'explosion' at point pos  of size radius */
int sgt_explosion(struct dm_map *map, coord_t *pos, int radius, coord_t *grid_list[]);

/*
   calculates a path from s to e.  path_list must be the address of a
   coord_t *, which will be malloc'ed by the funtion and must be
   freed by the caller unless the return value is < 0.
   continue_path specifies if the path must extend past point e,
   always up and until an obstruction.  it returns the length of the path.
 */
int sgt_los_path(struct dm_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path);

/* true if point e is visible from point s.  */
bool sgt_has_los(struct dm_map *map, coord_t *s, coord_t *e, int radius);

/* true if point e is visible and there are no obstructions from point s. */
bool sgt_has_lof(struct dm_map *map, coord_t *s, coord_t *e, int radius);

bool sgt_in_radius(struct dm_map *map, coord_t *s, coord_t *e, int radius);

#endif /*SIGHT_H_*/
