#include <limits.h>
#include <stdint.h>
#include <assert.h>

#include "rpsc_fov.h"

#include "heresyrl_def.h"

enum rpsc_octant {
    OCTANT_NNW,
    OCTANT_NWW,

    OCTANT_SWW,
    OCTANT_SSW,

    OCTANT_SSE,
    OCTANT_SEE,

    OCTANT_NEE,
    OCTANT_NNE,

    OCTANT_MAX,
};

struct rpsc_octant_quad {
    int x;
    int y;
    bool flip;
    const char *desc;
};

struct rpsc_octant_quad octant_lo_table[OCTANT_MAX] = {
    [OCTANT_NNW] = { .x = -1, .y = -1, .flip = false, .desc = "north north west", },
    [OCTANT_NWW] = { .x = -1, .y = -1, .flip = true,  .desc = "north west west",  },
    [OCTANT_SWW] = { .x = -1, .y =  1, .flip = true,  .desc = "south west west",  },
    [OCTANT_SSW] = { .x = -1, .y =  1, .flip = false, .desc = "south south west", },
    [OCTANT_SSE] = { .x =  1, .y =  1, .flip = false, .desc = "south south east", },
    [OCTANT_SEE] = { .x =  1, .y =  1, .flip = true,  .desc = "south east east",  },
    [OCTANT_NNE] = { .x =  1, .y = -1, .flip = false, .desc = "north north east", },
    [OCTANT_NEE] = { .x =  1, .y = -1, .flip = true,  .desc = "north east east",  },
};

typedef uint_fast32_t angle_t;
//#define FP_MAX UINT_FAST32_MAX
#define FP_MAX UINT16_MAX
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
    angle_t range = (max_range / (row +1) );
    set.near = range * cell;
    set.far  = set.near + range;
    set.center = set.near + (range / 2);

    lg_debug("as[%d,%d], (%d,%d,%d)", row,cell, set.near,set.center,set.far);
    assert(set.near < set.center);
    assert(set.center < set.far);
    return set;
}

static inline int angle_set_to_cell(struct angle_set *set, int row_new) {
    angle_t new_range = ANGLE_RANGE / (row_new +1);
    return set->center / new_range;
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
    return true;
}

static int scrub_blocked_list(struct angle_set *list, int list_sz) {
    int max_not_scrubbed = -1;

    lg_debug("scrub start, list sz is %d", list_sz);

    for (int i = list_sz -1; i >= 0; i--) {
        struct angle_set *a = &list[i];
        bool combined = false;

        for (int j = 0; (j < list_sz -1) && (combined == false); j++) {
            if (i == j) continue;

            if ( ( (a->near >= list[j].near) && (a->near <= list[j].far) ) ||
                 ( (a->far <= list[j].far)   && (a->far >= list[j].near) ) ) {
                extend_block(&list[j], a);
                list_sz--;
                combined = true;
                lg_debug("combining [%d] (%d,%d,%d) with [%d] (%d,%d,%d)", i, a->near, a->center, a->far, j, list[j].near, list[j].center, list[j].far);
            }
        }

        if ( (combined == false) && (max_not_scrubbed < i) ) max_not_scrubbed = i+1;
    }

    lg_debug("scrub end, list sz is %d", MAX(list_sz, max_not_scrubbed));
    return MAX(list_sz, max_not_scrubbed);
}

static void rpsc_fov_octant(struct rpsc_fov_set *set, coord_t *src, int radius, enum rpsc_octant octant, angle_t min_angle, angle_t max_angle) {
    struct angle_set blocked_list[radius *2];
    int obstacles_total = 0;
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];

    lg_debug("-------------start in octand %s----------", oct_mod->desc);

    if (min_angle > 0) {
        struct angle_set a = { .near = 0, .center = min_angle /2, .far = min_angle, };
        blocked_list[obstacles_total++] = a;
    }
    if (max_angle < ANGLE_RANGE) {
        struct angle_set a = { .near = max_angle, .center = max_angle + (ANGLE_RANGE - max_angle) /2, .far = ANGLE_RANGE, };
        blocked_list[obstacles_total++] = a;
    }

    for (int row = 1; row <= radius; row++) {
        int obstacles_this_line = 0;
        int row_max = row+1;

        angle_t angle_min = 0;
        angle_t angle_max = ANGLE_RANGE;

        for (int cell = 0; cell < row_max; cell++) {
            coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
            if (oct_mod->flip) {
                point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
            }

            lg_debug("row %d, cell %d, point (%d,%d)", row, cell, point.x,point.y);

            if (cd_within_bound(&point, &set->size) ) {
                struct angle_set as = offset_to_angle_set(row, cell);
                bool blocked = false;

                if (cell == 0) angle_min = as.near;
                if (cell == row) angle_max = as.far;

                for (int i = 0; (i < obstacles_total) && (blocked == false); i++) {
                    lg_debug("test (%d,%d,%d) vs [%d] (%d,%d,%d)", as.near, as.center, as.far, i, blocked_list[i].near, blocked_list[i].center, blocked_list[i].far);
                    if (angle_is_blocked(set, &as, &blocked_list[i]) ) {

                        lg_debug("blocked by [%d]", i);
                        if (set->not_visible_blocks_vision) {
                            blocked_list[obstacles_total + obstacles_this_line] = as;
                            obstacles_this_line++;
                            lg_debug("becomes obstacle [%d]", obstacles_this_line + obstacles_total -1);
                        }
                        else lg_debug("but does not block vision");

                        blocked = true;
                    }
                }

                if (blocked == false) {
                    if ( ( ( (row) + (cell/2) ) ) <= radius) {
                        if (set->apply != NULL) set->apply(set, &point, src);
                    }

                    if (set->is_opaque(set, &point, src) == false) {
                        blocked_list[obstacles_total + obstacles_this_line] = as;
                        obstacles_this_line++;
                        lg_debug("visible obstacle [%d]", obstacles_this_line + obstacles_total -1);
                    }
                    else lg_debug("visible floor");
                }
            }
        }
    
        obstacles_total = scrub_blocked_list(blocked_list, obstacles_total + obstacles_this_line);
        if ( (obstacles_total == 1) && (blocked_list[0].near <= angle_min) && (blocked_list[0].far >= angle_max) ) {
            lg_debug("done: %d,%d", blocked_list[0].near, blocked_list[0].far);
            return;
        }
    }
    return;
}

void rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius) {
    if (set->apply != NULL) set->apply(set, src, src);

    lg_debug("fov start src: (%d,%d)", src->x,src->y);
    for (int i = 0; i < OCTANT_MAX; i++) {
        rpsc_fov_octant(set,src,radius, i, 0, ANGLE_RANGE);
    }
}

void rpsc_cone(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, int angle, int radius) {
    enum rpsc_octant octant = get_octant(src, dst);
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];
    int nr_octants = angle / 45;

    coord_t delta = cd_delta_abs(src, dst);
    int cell_dst = delta.x;
    int row_dst = delta.y;
    if (oct_mod->flip) {
        cell_dst = delta.y;
        row_dst = delta.x;
    }
    struct angle_set as_dst = offset_to_angle_set(row_dst, cell_dst);
}

bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst) {
    enum rpsc_octant octant = get_octant(src, dst);
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];
    bool visible = true;

    struct angle_set blocked_list[cd_pyth(src,dst) *2];
    int obstacles_total = 0;
    int obstacles_this_line = 0;

    if (set->apply != NULL) lg_debug("-------------with apply start in octand %s----------", oct_mod->desc);
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

    for (int row = 1; (row <= row_dst) && (visible == true); row++) {
        int center_cell = angle_set_to_cell(&as_dst, row);

        int cell_select[3] = {0,-1,1};

        bool applied = false;
        obstacles_this_line = 0;
        for (int c = 0; c <= 2; c++) {
            int cell = center_cell +cell_select[c];
            lg_debug(">>> row %d, cell %d, c: %d", row, cell, cell_select[c]);
            if (cell < 0) continue;
            if (cell > row) continue;
            if (cell_select[c] != 0 && row == row_dst) continue;

            coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
            if (oct_mod->flip) {
                point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
            }
            lg_debug("next point is (%d,%d)", point.x,point.y);

            struct angle_set as = offset_to_angle_set(row, cell);
            bool blocked = false;
            for (int i = 0; i < obstacles_total; i++) {
                lg_debug("test (%d,%d,%d) vs [%d] (%d,%d,%d)", as.near, as.center, as.far, i, blocked_list[i].near, blocked_list[i].center, blocked_list[i].far);
                if (angle_is_blocked(set, &as, &blocked_list[i]) ) {

                    lg_debug("blocked by [%d]", i);
                    if (set->not_visible_blocks_vision) {
                        blocked_list[obstacles_total + obstacles_this_line] = as;
                        obstacles_this_line++;
                        lg_debug("becomes obstacle [%d]", obstacles_this_line + obstacles_total -1);
                    }
                    else lg_debug("but does not block vision");

                    blocked = true;
                }
            }

            if (blocked == false) {
                if (set->is_opaque(set, &point, src) == false) {
                    blocked_list[obstacles_total + obstacles_this_line] = as;
                    obstacles_this_line++;
                    lg_debug("becomes obstacle [%d]", obstacles_this_line + obstacles_total -1);
                    blocked = true;
                }
                else if (set->apply != NULL && applied == false) {
                    set->apply(set, &point, src);
                    applied = true;
                }
            }

            if (blocked == true) {
                if (cd_equal(&point,dst) ) {
                    visible = false;
                }
            }
        }

        obstacles_total = scrub_blocked_list(blocked_list, obstacles_total + obstacles_this_line);
        if ( (obstacles_total == 1) && (blocked_list[0].near <= as_dst.near) && (blocked_list[0].far >= as_dst.far) ) {
            visible = false;
        }
    }

   if (set->apply != NULL) lg_debug("-------------with apply end, visibility is %s----------", (visible) ? "true": "false");
    return visible;
}

