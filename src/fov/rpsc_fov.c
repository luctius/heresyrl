#include <limits.h>
#include <stdint.h>
#include <assert.h>

#include "rpsc_fov.h"

#include "heresyrl_def.h"

typedef uint_fast32_t angle_t;
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
    struct angle_set set;
    angle_t range = ANGLE_RANGE / (offset->y +1);
    set.near = range * offset->x;
    set.far  = set.near + range;
    set.center = set.near + (range / 2);
    return set;
}

static inline coord_t angle_set_to_coord_t(struct angle_set *set, int y_offset_new) {
    angle_t new_range = ANGLE_RANGE / (y_offset_new +1);
    coord_t c = { .y = y_offset_new, };
    c.x = set->center / new_range;
    if ( (set->center - c.x) > (new_range / 2) ) c.x += 1;
    return c;
}

inline static bool angle_within_angle_set(struct angle_set *test_set, struct angle_set *blocked_set) {
    lg_debug("test (%d,%d,%d), blocked (%d,%d,%d)", test_set->near, test_set->center, test_set->far, blocked_set->near, blocked_set->center, blocked_set->far);

    if (test_set->far < blocked_set->near) return false;
    if (test_set->near > blocked_set->far) return false;
    if ( (test_set->center < blocked_set->near) || (test_set->center > blocked_set->far) ) return false;
    return true;
}

static bool diag_blocked = false;
static bool rpsc_fov_quadrant(struct rpsc_fov_set *set, coord_t *src, int radius) {
    struct angle_set blocked_list[radius +1 + 200];

    int obstacles_total = 0;
    int obstacles_last_line = 0;

    for (int row = 1; row < (radius +1); row++) {
        int obstacles_this_line = 0;

        for (int cell = 0; cell < (row +1); cell++) {
            coord_t point = cd_create(src->x - cell, src->y - row);
            coord_t delta = cd_delta(src, &point);

            if (cd_within_bound(&point, &set->size) ) {
                struct angle_set as = offset_to_angle_set(&delta);
                bool blocked = false;
                lg_debug("angle_set (%d,%d,%d)", as.near, as.center, as.far);

                lg_debug("testing point (%d,%d) ll: %d tot: %d", point.x, point.y, obstacles_last_line, obstacles_total);

                for (int i = 0; (i < obstacles_last_line) && (blocked == false); i++) {
                    if (angle_within_angle_set(&as, &blocked_list[i]) ) {
                        blocked = true;
                        lg_debug("blocked (%d,%d)", point.x, point.y);
                    }
                }

                if (blocked == false) {
                    set->apply(set, &point, src);

                    if (set->is_opaque(set, &point, src) == false) {
                        blocked_list[obstacles_total + obstacles_this_line] = as;
                        obstacles_this_line++;
                        lg_debug("point (%d,%d) is obstacle", point.x, point.y);
                    }
                }
            }

        }
        obstacles_total += obstacles_this_line;
        obstacles_last_line = obstacles_total;
    }
    return true;
}

bool rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius) {
    lg_debug("----------------------------------");
    rpsc_fov_quadrant(set,src,radius);
    lg_debug("----------------------------------");
    return true;
}

bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, bool apply) {
    return false;
}
