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

#ifndef COORD_H
#define COORD_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>
#include <sys/param.h>

typedef struct coord {
    int x;
    int y;
} coord_t;

extern const coord_t coord_nhlo_table[];
extern const uint8_t coord_nhlo_table_sz;

inline coord_t cd_create(int x, int y) {
    coord_t c = { .x = x, .y = y,};
    return c;
}

inline coord_t cd_add(const coord_t *a, const coord_t *b) {
    coord_t c = { .x = a->x + b->x, .y = a->y + b->y,};
    return c;
}

inline bool cd_equal(const coord_t *a, const coord_t *b) {
    if (a->x != b->x) return false;
    if (a->y != b->y) return false;
    return true;
}

inline bool cd_within_bound(const coord_t *a, const coord_t *b) {
    if (a->x < 0) return false;
    if (a->y < 0) return false;
    if (a->x >= b->x) return false;
    if (a->y >= b->y) return false;
    return true;
}

inline int cd_pyth(const coord_t *a, const coord_t *b) {
    int s1 = abs(a->x - b->x);
    int s2 = abs(a->y - b->y);
    return MAX(s1, s2);
}

inline coord_t cd_delta(const coord_t *a, const coord_t *b) {
    coord_t c;
    c.x = a->x - b->x;
    c.y = a->y - b->y;
    return c;
}

inline coord_t cd_delta_abs(const coord_t *a, const coord_t *b) {
    coord_t c;
    c.x = abs(a->x - b->x);
    c.y = abs(a->y - b->y);
    return c;
}

inline int cd_delta_abs_total(const coord_t *a, const coord_t *b) {
    coord_t c = cd_delta_abs(a,b);
    return c.x + c.y;
}

inline bool cd_neighbour(const coord_t *a, const coord_t *b) {
    coord_t c = cd_delta_abs(a,b);
    if (c.x > 1) return false;
    if (c.y > 1) return false;
    return true;
}

#endif /* COORD_H */
