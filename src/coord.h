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
    short x;
    short y;
} coord_t;

inline coord_t cd_create(short x, short y) {
    coord_t c = { .x = x, .y = y,};
    return c;
}

inline coord_t cd_add(const coord_t *a, const coord_t *b) {
    coord_t c = { .x = a->x + b->x, .y = a->y + a->y,};
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

inline short cd_pyth(const coord_t *a, const coord_t *b) {
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
