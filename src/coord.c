#include "coord.h"

extern inline coord_t cd_create(int x, int y);
extern inline coord_t cd_add(coord_t *a, coord_t *b);
extern inline bool cd_equal(coord_t *a, coord_t *b);
extern inline bool cd_within_bound(coord_t *a, coord_t *max);
extern inline int cd_pyth(coord_t *a, coord_t *b);
