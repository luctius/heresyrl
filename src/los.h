#ifndef LOS_H
#define LOS_H

#include "heresyrl_def.h"
#include "coord.h"

int lof_calc_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);
bool los_has_sight(coord_t *s, coord_t *e, struct dc_map *map);
bool los_has_lof(coord_t *s, coord_t *e, struct dc_map *map);

#endif /* LOS_H */
