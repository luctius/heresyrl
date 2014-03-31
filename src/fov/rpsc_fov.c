#include <limits.h.h>
#include <stdint.h>

#include "rpsc_fov.h"

typedef uint_fast32_t angle_t
#define ANGLE_MAX UINT_FAST16_MAX
#define FP_MAX 0xFFFF
#define FP_HALF (FP_MAX>>1)
#define ANGLE_RANGE FP_HALF

struct angle_set {
    angle_t near;
    angle_t center;
    angle_t far;
};

static inline struct angle_set offset_to_angle_set(coord_t *offset) {
    assert(offset->x <= (offset->y+1) ); /* this only works for one quadrant, fix that! */

    struct angle_set a;
    angle_t range = ANGLE_RANGE / (offset->y +1);
    a.near = range * offset->x;
    a.far  = a.near + range;
    a.center = a.near + (range / 2);
}

static inline coord_t angle_set_to_coord_t(struct angle_set *a, int row) {
}

static bool diag_blocked = false;
static bool rpsc fov_quadrant(struct rpsc_fov_set *set, coord_t *src, int radius) {
    struct angle_set blocked_list[radius +1];

    int obstacles_total = 0;
    int obstacles_last_line = 0;

    for (int row = 0; row < (radius +1); row++) {
        for (int cell = 0; cell < (row +1); cell++) {
        }
    }
}

bool rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius) {

}

bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, bool apply);
