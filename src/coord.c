#include "coord.h"

extern inline coord_t cd_create(short x, short y);
extern inline coord_t cd_add(const coord_t *a, const coord_t *b);
extern inline bool cd_equal(const coord_t *a, const coord_t *b);
extern inline bool cd_within_bound(const coord_t *a, const coord_t *max);
extern inline short cd_pyth(const coord_t *a, const coord_t *b);
extern inline coord_t cd_delta(const coord_t *a, const coord_t *b);
extern inline coord_t cd_delta_abs(const coord_t *a, const coord_t *b);
extern inline int cd_delta_abs_total(const coord_t *a, const coord_t *b);
extern inline bool cd_neighbour(const coord_t *a, const coord_t *b);
