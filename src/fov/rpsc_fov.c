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
#define FP_MAX UINT_FAST32_MAX
#define FP_HALF ( (FP_MAX>>1) & ~0xF)
#define ANGLE_RANGE FP_HALF

struct angle_set {
    angle_t near;
    angle_t center;
    angle_t far;
};

enum rpsc_octant get_octant(coord_t *src, coord_t *dst) {
    coord_t delta = cd_delta_abs(src, dst);

    for (int i = 0; i < OCTANT_MAX; i++) {
        struct rpsc_octant_quad *oct_mod = &octant_lo_table[i];
        bool flip = (delta.x >= delta.y);
        if ( (src->x + (delta.x * oct_mod->x) == dst->x) && 
             (src->y + (delta.y * oct_mod->y) == dst->y) ) {
            if (flip == oct_mod->flip) {
                return i;   
            }
        }
    }
    return 0;
}

static inline struct angle_set offset_to_angle_set(int row, int cell) {
    struct angle_set set;
    angle_t max_range = ANGLE_RANGE;
    angle_t range = max_range / (row +1);
    set.near = range * cell;
    set.far  = set.near + range;
    set.center = set.near + (range / 2);

    assert(set.near < set.center);
    assert(set.center < set.far);
    assert(set.near < set.far);
    return set;
}

static inline int angle_set_to_cell(struct angle_set *set, int row_new, bool flip) {
    angle_t new_range = ANGLE_RANGE / (row_new +1);
    int cell = set->center / new_range;
    //if ( (set->center - cell) > (new_range / 2) ) cell += 1;
    return cell;
}

inline static bool angle_is_blocked(struct rpsc_fov_set *set, struct angle_set *test_set, struct angle_set *blocked_set) {
    if (test_set->far < blocked_set->near) return false;
    if (test_set->near > blocked_set->far) return false;

    bool near_blocked = false;
    bool center_blocked = false;
    bool far_blocked = false;

    if (set->visible_on_equal) {
        if ( (test_set->near   > blocked_set->near) && (test_set->near   < blocked_set->far) ) near_blocked = true;
        if ( (test_set->center > blocked_set->near) && (test_set->center < blocked_set->far) ) center_blocked = true;
        if ( (test_set->far    > blocked_set->near) && (test_set->far    < blocked_set->far) ) far_blocked = true;
    }
    else {
        if ( (test_set->near   >= blocked_set->near) && (test_set->near   <= blocked_set->far) ) near_blocked = true;
        if ( (test_set->center >= blocked_set->near) && (test_set->center <= blocked_set->far) ) center_blocked = true;
        if ( (test_set->far    >= blocked_set->near) && (test_set->far    <= blocked_set->far) ) far_blocked = true;
    }

    if (set->permissiveness == RPSC_FOV_PERMISSIVE_RELAXED) {
        return (near_blocked && center_blocked && far_blocked);
    }
    else if (set->permissiveness == RPSC_FOV_PERMISSIVE_STRICT) {
        return (near_blocked || center_blocked || far_blocked);
    }

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

    assert(blocked_set->near < blocked_set->center);
    assert(blocked_set->center < blocked_set->far);
    assert(blocked_set->near < blocked_set->far);
    return true;
}

static int scrub_blocked_list(struct angle_set *list, int list_sz) {
    bool scrubbed[list_sz];
    int max_not_scrubbed = -1;

    for (int i = list_sz -1; i >= 0; i--) {
        struct angle_set *a = &list[i];
        bool combined = false;
        scrubbed[i] = false;

        for (int j = 0; (j < list_sz -1) && (combined == false); j++) {
            if (i == j) continue;

            if ( ( (a->near >= list[j].near) && (a->near <= list[j].far) ) ||
                 ( (a->far <= list[j].far)   && (a->far >= list[j].near) ) ) {
                extend_block(&list[j], a);
                list_sz--;
                combined = true;
            }
        }

        if (combined == true) {
            scrubbed[i] = true;
        }
        else if (max_not_scrubbed < i) max_not_scrubbed = i;
    }

    /* out list may be discontinues, move non-scrubbed angle sets to the front */
    if (max_not_scrubbed >= list_sz) {
        for (int i = max_not_scrubbed; i >= 0; i--) {
            if (scrubbed[i] == false) {
                bool moved = false;

                for (int j = 0; (j < list_sz) && (moved == false); j++) {
                    if (scrubbed[j] == true) {
                        list[j] = list[i];
                        scrubbed[j] = false;
                        moved = true;
                    }
                }
            }
        }
    }

    return list_sz;
}

static void rpsc_fov_octant(struct rpsc_fov_set *set, coord_t *src, int radius, enum rpsc_octant octant) {
    struct angle_set blocked_list[radius *2];
    int obstacles_total = 0;
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];

    for (int row = 1; row <= radius; row++) {
        int obstacles_this_line = 0;
        int nr_blocked = 0;
        int row_max = row+1;
        if (oct_mod->flip == true) row_max = row;

        for (int cell = 0; cell < row_max; cell++) {
            int delta_cell_sqr = cell*cell;
            coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
            if (oct_mod->flip) {
                point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
            }

            if (cd_within_bound(&point, &set->size) ) {
                struct angle_set as = offset_to_angle_set(row, cell);
                bool blocked = false;

                for (int i = 0; (i < obstacles_total) && (blocked == false); i++) {
                    if (angle_is_blocked(set, &as, &blocked_list[i]) ) {
                        if (set->not_visible_blocks_vision) {
                            if (set->is_opaque(set, &point, src) == false) {
                                blocked_list[obstacles_total + obstacles_this_line] = as;
                                obstacles_this_line++;
                            }
                        }

                        blocked = true;
                        nr_blocked++;
                    }
                }

                if (blocked == false) {
                    if ( ( ( (row) + (cell/2) ) ) <= radius) {
                        if (set->apply != NULL) set->apply(set, &point, src);
                    }

                    if (set->is_opaque(set, &point, src) == false) {
                        blocked_list[obstacles_total + obstacles_this_line] = as;
                        obstacles_this_line++;
                    }
                }
            }
            else nr_blocked++;
        }
    
        obstacles_total = scrub_blocked_list(blocked_list, obstacles_total + obstacles_this_line);
        if ( (obstacles_total == 1) && (blocked_list[0].near == 0) && (blocked_list[0].far == ANGLE_RANGE) ) {
            return;
        }
    }
    return;
}

void rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius) {
    if (set->apply != NULL) set->apply(set, src, src);

    //rpsc_fov_octant(set,src,radius, OCTANT_SWW);
    for (int i = 0; i < OCTANT_MAX; i++) {
        rpsc_fov_octant(set,src,radius, i);
    }
}

bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst) {
    enum rpsc_octant octant = get_octant(src, dst);
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];
    bool visible = true;

    if (set->apply != NULL) lg_debug("-------------with apply start in octand %d----------", octant);
    if (set->apply != NULL) set->apply(set, src, src);

    coord_t delta = cd_delta_abs(src, dst);
    int cell_dst = delta.x;
    int row_dst = delta.y;
    if (oct_mod->flip) {
        cell_dst = delta.y;
        row_dst = delta.x;
    }

    struct angle_set as_dst = offset_to_angle_set(row_dst, cell_dst);

    lg_debug("los: (%d,%d) -> (%d,%d), length: %d", src->x,src->y,dst->x,dst->y, row_dst);

    for (int row = 1; (row <= row_dst-1) && (visible == true); row++) {
        int cell = angle_set_to_cell(&as_dst, row, oct_mod->flip);
        lg_debug("row %d, cell %d", row, cell);

        struct angle_set as = offset_to_angle_set(row, cell);

        coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
        if (oct_mod->flip) {
            point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
        }

        if (cd_equal(&point, src) ) continue;

        lg_debug("next point is (%d,%d)", point.x,point.y);
        if (set->apply != NULL) set->apply(set, &point, src);

        if (set->is_opaque(set, &point, src) == false) {
            lg_debug("(%d,%d) is an obstacle", point.x,point.y);
            //if (angle_is_blocked(set, &as_dst, &as) ) {
                visible = false;
            //}
        }
    }

    if (set->apply != NULL) set->apply(set, dst, src);
    if (set->apply != NULL) lg_debug("-------------with apply end----------");

    return visible;
}

