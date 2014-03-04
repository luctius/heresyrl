#ifndef COORD_H
#define COORD_H

#include <stdbool.h>
#include <math.h>
#include <heresyrl_def.h>

typedef struct coord {
    int x;
    int y;
} coord_t;

inline coord_t cd_create(int x, int y) {
    coord_t c = { .x = x, .y = y,};
    return c;
}

inline coord_t cd_add(coord_t *a, coord_t *b) {
    coord_t c = { .x = a->x + b->x, .y = a->y + a->y,};
    return c;
}

inline bool cd_equal(coord_t *a, coord_t *b) {
    if (a->x != b->x) return false;
    if (a->y != b->y) return false;
    return true;
}

inline bool cd_within_bound(coord_t *a, coord_t *b) {
    if (a->x < 0) return false;
    if (a->y < 0) return false;
    if (a->x >= b->x) return false;
    if (a->y >= b->y) return false;
    return true;
}

inline int cd_pyth(coord_t *a, coord_t *b) {
    int s1 = abs(a->x - b->x);
    int s2 = abs(a->y - b->y);
    return pyth(s1, s2);
}

#endif /* COORD_H */
