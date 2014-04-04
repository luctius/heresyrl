#include "coord.h"

extern inline coord_t cd_create(short x, short y);
extern inline coord_t cd_add(coord_t *a, coord_t *b);
extern inline bool cd_equal(coord_t *a, coord_t *b);
extern inline bool cd_within_bound(coord_t *a, coord_t *max);
extern inline short cd_pyth(coord_t *a, coord_t *b);
extern inline coord_t cd_delta(coord_t *a, coord_t *b);
extern inline coord_t cd_delta_abs(coord_t *a, coord_t *b);
extern inline int cd_delta_abs_total(coord_t *a, coord_t *b);
extern inline bool cd_neighbour(coord_t *a, coord_t *b);
