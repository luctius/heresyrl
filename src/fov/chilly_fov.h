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

#ifndef CHILLY_FOV_H
#define CHILLY_FOV_H

#include <stdbool.h>

#include "coord.h"

enum chl_area {
    CHL_AREA_SQUARE,
    CHL_AREA_OCTAGON,
    CHL_AREA_CIRCLE,
    CHL_AREA_CIRCLE_STRICT,
};

struct chl_fov;
struct chl_fov_set {
    /* the item or actor which 'sees' the fov*/
    void *source;

    /* map structure. */
    void *map;

    /* size in the x and y direction */
    coord_t size;

    /* fov area */
    enum chl_area area;

    /* callback, should return true if the tile does not block vision, false if it does. */
    bool (*is_transparent)(struct chl_fov_set *set, coord_t *point, coord_t *origin);

    /* callback, applies the line of sight. */
    bool (*apply)(struct chl_fov_set *set, coord_t *point, coord_t *origin);
};

struct chl_fov *chl_fov_init(int max_radius);
void chl_fov_exit();

/* creates a fov from src with a given radius.
   returns true, unless there is an error with the given parameters.  */
bool chl_fov(struct chl_fov_set *set, coord_t *src, int radius);

/* creates a cone from src with dst in the middle of angle and a given radius.
   returns true, unless there is an error with the given parameters. */
bool chl_cone(struct chl_fov_set *set, coord_t *src, coord_t *dst, int angle, int radius);

/*
   calculates line of sight from src to dst.
   returns true if src can see dst.
   if apply within set is not NULL, it will give the path from src to dst.
   This path will be created on the go, thus when it return with false, the path should be ignored.
 */
bool chl_los(struct chl_fov_set *set, coord_t *src, coord_t *dst);

/* check if the point dst is within radius of src given the current area settings and radius. */
bool chl_in_radius(struct chl_fov_set *set, coord_t *src, coord_t *dst, int radius);

#endif /* CHILLY_FOV_H */
