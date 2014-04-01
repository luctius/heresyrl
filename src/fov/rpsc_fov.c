#include <limits.h>
#include <stdint.h>
#include <assert.h>

#include "rpsc_fov.h"

#include "heresyrl_def.h"

enum rpsc_octant {
    OCTANT_NNW,
    OCTANT_NWW,

    OCTANT_SSW,
    OCTANT_SWW,

    OCTANT_SEE,
    OCTANT_SSE,

    OCTANT_NNE,
    OCTANT_NEE,

    OCTANT_MAX,
};

struct rpsc_octant_quad {
    int x;
    int y;
    bool flip;
};

struct rpsc_octant_quad octant_lo_table[OCTANT_MAX] = {
    [OCTANT_NNW] = { .x = -1, .y = -1, .flip = false, },
    [OCTANT_NWW] = { .x = -1, .y = -1, .flip = true,  },
    [OCTANT_SWW] = { .x = -1, .y =  1, .flip = false, },
    [OCTANT_SSW] = { .x = -1, .y =  1, .flip = true,  },
    [OCTANT_SSE] = { .x =  1, .y =  1, .flip = false, },
    [OCTANT_SEE] = { .x =  1, .y =  1, .flip = true,  },
    [OCTANT_NEE] = { .x =  1, .y = -1, .flip = false, },
    [OCTANT_NNE] = { .x =  1, .y = -1, .flip = true,  },
};

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

enum rpsc_octant get_octant(coord_t *src, coord_t *dst) {
    coord_t delta = cd_delta_abs(src, dst);

    for (int i = 0; i < OCTANT_MAX; i+=2) {
        struct rpsc_octant_quad *oct_mod = &octant_lo_table[i];
        if ( (src->x + (delta.x * oct_mod->x) == dst->x) && 
             (src->y + (delta.y * oct_mod->y) == dst->y) ) {
            if (delta.x <= delta.y) return i;
            return i+1;
        }
    }
    return 0;
}

static inline struct angle_set offset_to_angle_set(int row, int cell) {
    struct angle_set set;
    angle_t range = ANGLE_RANGE / (row +1);
    set.near = range * cell;
    set.far  = set.near + range;
    set.center = set.near + (range / 2);
    return set;
}

static inline int angle_set_to_cell(struct angle_set *set, int row_new, bool flip) {
    angle_t new_range = ANGLE_RANGE / (row_new +1);
    int cell = set->center / new_range;
    if ( (set->center - cell) > (new_range / 2) ) cell += 1;
    return cell;
}

inline static bool angle_is_blocked(struct angle_set *test_set, struct angle_set *blocked_set) {
    lg_debug("test (%d,%d,%d), %d, blocked (%d,%d,%d)", test_set->near, test_set->center, test_set->far, blocked_set->near, blocked_set->center, blocked_set->far);

    if (test_set->far < blocked_set->near) return false;
    if (test_set->near > blocked_set->far) return false;

    bool near_blocked = false;
    bool center_blocked = false;
    bool far_blocked = false;

    if ( (test_set->near > blocked_set->near) && (test_set->near < blocked_set->far) ) near_blocked = true;
    if ( (test_set->center > blocked_set->near) && (test_set->center < blocked_set->far) ) center_blocked = true;
    if ( (test_set->far > blocked_set->near) && (test_set->far < blocked_set->far) ) far_blocked = true;

    return (near_blocked && center_blocked) || (center_blocked && far_blocked);
}

inline static bool cd_in_radius(coord_t *p, coord_t *src, int radius) {
    coord_t delta = cd_delta(p, src);
    if (hypot(delta.x, delta.y) <= radius) return true;
    return false;
}

inline static bool extend_block(struct angle_set *blocked_set, struct angle_set *current_set) {
    /* extend block */
    if (current_set->near < blocked_set->near) blocked_set->near = current_set->near;
    if (current_set->far > blocked_set->far) blocked_set->far = current_set->far;
    return true;
}

inline static bool point_in_radius(coord_t *src, coord_t *p, int radius) {
    float dis = sqrt( ( (src->x - p->x) * (src->x - p->x) ) + ( (src->y - p->y) * (src->y - p->y) ) );
    lg_debug("dis: %f, radius: %f", dis, (radius + 1/3.0f) );
    return dis <= (radius );
}

static void rpsc_fov_octant(struct rpsc_fov_set *set, coord_t *src, int radius, enum rpsc_octant octant) {
    struct angle_set blocked_list[radius +1];

    lg_debug("src (%d,%d)", src->x, src->y);

    int obstacles_total = 0;
    int obstacles_last_line = 0;
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];

    for (int row = 1; row <= radius; row++) {
        int obstacles_this_line = 0;
        int nr_blocked = 0;
        int row_max = row+1;

        for (int cell = 0; cell < row_max; cell++) {
            int delta_cell_sqr = cell*cell;
            coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
            if (oct_mod->flip) {
                point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
            }

            if (cd_within_bound(&point, &set->size) && ( (row>>1) + cell <= radius) ) {
                struct angle_set as = offset_to_angle_set(row, cell);
                bool blocked = false;

                lg_debug("testing point (%d,%d) ll: %d tot: %d", point.x, point.y, obstacles_last_line, obstacles_total);
                lg_debug("angle_set (%d,%d,%d)", as.near, as.center, as.far);

                for (int i = 0; (i < obstacles_last_line) && (blocked == false); i++) {
                    if (angle_is_blocked(&as, &blocked_list[i]) ) {
                        extend_block(&blocked_list[i], &as);
                        blocked = true;
                        nr_blocked++;
                        lg_debug("blocked (%d,%d)", point.x, point.y);
                    }
                }

                if (blocked == false) {
                    if (set->apply != NULL) set->apply(set, &point, src);

                    if (set->is_opaque(set, &point, src) == false) {
                        blocked_list[obstacles_total + obstacles_this_line] = as;
                        obstacles_this_line++;
                        lg_debug("point (%d,%d) is obstacle", point.x, point.y);
                    }
                }
            }
            else nr_blocked++;

            if ( (nr_blocked + obstacles_this_line) == row_max) {
                lg_debug("all point are blocked, bailing");
                return;
            }
        }
        obstacles_total += obstacles_this_line;
        obstacles_last_line = obstacles_total;
    }
    return;
}

void rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius) {
    if (set->apply != NULL) set->apply(set, src, src);

    for (int i = 0; i < OCTANT_MAX; i++) {
        lg_debug("----------------------------------");
        rpsc_fov_octant(set,src,radius, i);
        lg_debug("----------------------------------");
    }
}

bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst) {
    enum rpsc_octant octant = get_octant(src, dst);
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];
    bool visible = true;

    coord_t delta = cd_delta_abs(src, dst);
    int cell_dst = delta.x;
    int row_dst = delta.y;
    if (oct_mod->flip) {
        cell_dst = delta.y;
        row_dst = delta.x;
    }

    struct angle_set as_dst = offset_to_angle_set(row_dst, cell_dst);

    for (int row = 0; (row <= row_dst) && (visible == true); row++) {
        int cell = angle_set_to_cell(&as_dst, row, oct_mod->flip);
        struct angle_set as = offset_to_angle_set(row, cell);

        coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
        if (oct_mod->flip) {
            point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
        }

        if (set->apply != NULL) set->apply(set, &point, src);

        if (set->is_opaque(set, &point, src) == false) {
            if (angle_is_blocked(&as_dst, &as) ) {
                visible = false;
            }
        }
    }

    return visible;
}

