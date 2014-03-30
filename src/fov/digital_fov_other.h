#ifndef __DIGITAL_FOV_OTHER_H__
#define __DIGITAL_FOV_OTHER_H__

#include "digital_fov.h"

/* the digital FOV (Field Of Vision)
 * (http://roguebasin.roguelikedevelopment.org/index.php?title=Digital_field_of_view)
 *
 * definition:
 * A line L(a, b, s) is a set of grids (x, y) such that
 * y = (b * x) / a + s, rounded down.
 * A grid (X, Y) in the first octant (that is, 0 <= Y <= X) can be seen
 * from the grid (0, 0) if and only if (X, Y) is (0, 0) or there are
 * real numbers a, b and s such that:
 * * 0 <= b / a <= 1
 * * L(a, b, s) passes (0, 0) and (X, Y)
 * * a grid (x, y) on L(a, b, s) is not a wall as long as 1 <= x <= X - 1
 */

/* map must be a 2-dimension array of size (map_size_x, map_size_y).
 * map[x][y] must be non-zero if the grid (x, y) is a wall, 0 otherwise.
 * map_fov must be a 2-dimension array of size
 * (2 * radius + 1, 2 * radius + 1).
 * The caller of the function must allocate enough memory to map_fov
 * before calling.
 * The result is written to map_fov; the grid (x, y) can be seen from
 * the grid (center_x, center_y) if and only if
 * map_fov[x - center_x + radius][y - center_y + radius] is non-zero.
 */

/* Field Of Vision
 * uses brute force
 * slow, only for debugging
 * runs at O(N^3)
 * return 0 on success, 1 on error
 */
/* int digital_fov_brute(int **map, int map_size_x, int map_size_y,
                      int **map_fov,
                      int center_x, int center_y, int radius); */

/* Field Of Vision
 * uses beam casting of width 1
 * (http://roguebasin.roguelikedevelopment.org/index.php?title=Digital_field_of_view_implementation)
 * runs at O(N^2), but slower than shadowcasting because a grid may be
 * visited more than once
 * return 0 on success, 1 on error
 */
/* int digital_fov_beam(int **map, int map_size_x, int map_size_y,
                     int **map_fov,
                     int center_x, int center_y, int radius); */

bool digital_fov_brute(struct digital_fov_set *set, coord_t *src, int radius);
bool digital_fov_beam(struct digital_fov_set *set, coord_t *src, int radius);

#endif /* not __DIGITAL_FOV_OTHER_H__ */
