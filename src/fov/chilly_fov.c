/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <limits.h>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>

//#define RPSC_DEBUG

#include "chl_fov.h"

#ifdef RPSC_DEBUG
#include "logging.h"
#else
#define lg_debug(a, ...) do { } while (0);
#endif

/*
   This is an implementation of:
    https://www.reddit.com/r/roguelikedev/comments/5n1tx3/fov_algorithm_sharencompare/
*/

/*  instead of using floating point, we use a 16bit integer (and more when available) */
typedef uint_fast16_t angle_t;

#ifdef RPSC_DEBUG
#define FP_MAX UINT16_MAX /* use this when debugging to avoid becoming number crazy */
#else
#define FP_MAX UINT_FAST16_MAX
#endif

/* We ignore rounding error in the last nibble */
#define PERIOD_MASK (~0xF)
#define ANGLE_RANGE (FP_MAX & PERIOD_MASK)

/* this describes the angles */
struct cell {
    coord_t pos;
    angle_t near;
    angle_t center;
    angle_t far;
    int distance;
};

struct chl_fov {
    int max_radius;
    int max_cells;
    struct cell *clist;
    uint8_t *empty;
    uint8_t *full;
};

/* calculate the angles for this cell */
static inline struct cell offset_to_angle_set(angle_t max_range, coord_t pos) {
    struct angle_set set;

    int row  = MAX(pos.x, pos.y);
    int cell = MIN(pos.x, pos.y);

    /* range per cell */
    angle_t range = (max_range / (row+1) ) / 8;

    /* starting angle */
    set.near = range * cell;

    /* far angle is equal to the starting angle of the next cell */
    set.far  = set.near + range;

    /* center angle is in between */
    set.center = set.near + (range / 2);

    set.distance = row +1;

    lg_debug("as[%d,%d], (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ")", pos.x,pos.y, set.near,set.center,set.far);

    /* some paranoia checks */
    assert(set.near < set.center);
    assert(set.center < set.far);

    /* return calculated set */
    return set;
}

/* check if a given angle set falls within the blocked set, given certain parameters */
inline static bool angle_is_blocked(struct chl_fov_set *set, struct cell *c, angle_t a, bool transparent) {

    /* if it falls completely outside the blocked set, our job is done */
    if ( (a < c->near) && (a > c->far) ) return false;
    return true;
}

inline static int angle_to_cell_idx(struct chl_fov_set *set, )

/* check if (row,cell) is within radius with the given settings */
inline static bool in_radius(struct chl_fov_set *set, int row, int cell, int radius) {
    switch (set->area) {
        case RPSC_AREA_OCTAGON:
            if ( ( ( (row) + (cell/2) ) ) <= radius) return true;
            break;
        case RPSC_AREA_CIRCLE:
            /* nicer circle */
            if ( ( (row*row) + (cell*cell) ) <= ((radius*radius) + radius) ) return true;
            break;
        case RPSC_AREA_CIRCLE_STRICT:
            /* pythagoras circle, which does not translate well to an ascii grid. */
            if ( ( (row*row) + (cell*cell) ) <= (radius*radius) ) return true;
            break;
        case RPSC_AREA_SQUARE:
            /* simple square, row is always bigger than the cell. */
            if (row <= radius) return true;
            break;
        default: assert(false); break;
    }

    return false;
}

struct chl_fov *chl_fov_init(int max_radius) {
    struct chl_fov *ctx = calloc(1, sizeof(struct chl_fov) );
    if (ctx == NULL) return NULL;

    ctx->max_radius = radius;
    int max_cells   = radius * ( (radius+1) / 2) * 8;
    ctx->max_bins   = (radius * 3);
    ctx->empty      = calloc(max_bins,  1);
    ctx->full       = calloc(max_bins,  1);
    ctx->clist      = calloc(max_cells, sizeof(struct cell) );
    memset(ctx->full, 0xFF, max_bins);

    for (int i = 0; i < max_cells; i++) {
        ctx->clist[i] = offset_to_angle_set(radius * 8 * 3, );
    }

    return ctx;
}

void chl_fov_exit() {

}

/* calculates the complete fov */
bool chl_fov(struct chl_fov *ctx, struct chl_fov_set *set, coord_t *src, int radius) {
}

/* calculate the los from one point to another.
   In essence we use the exact same algorithm as chl_octant,
   however with some additions to avoid calculating the complete octant.

   We know the angles of the destination cell (we can calculate that).
   Using that information we loop through the rows and check the cell in
   the direct line if it is blocked or itself a blocker. But since the
   angles can cross neighbouring cells, we also check its direct neighbours.
   We check the center cell first, because that is our ideal line, and if
   that one is not blocked, we inform the user about it. If it is blocked,
   we take one of the others and we don't really care which one. Since the
   center cell is allready on a direct path, we do not need to keep the
   information that we took another cell since,
   1) it borders o the next center cell and
   2) the center cell of the next row is probably directly above it anyway.
*/
bool chl_los(struct chl_fov_set *set, coord_t *src, coord_t *dst) {
    if (set == NULL) return false;
    if (set->is_transparent == NULL) return false;
    if (cd_within_bound(src, &set->size) == false) return false;
    if (cd_within_bound(dst, &set->size) == false) return false;
    bool visible = true;

    /* find out which octant we are in */
    enum chl_octant octant = get_octant(src, dst);
    /* get the octant describtor */
    struct chl_octant_quad *oct_mod = &octant_lo_table[octant];

    /* total number of obstacles found up untill the previous line */
    int obstacles_total = 0;
    /* maximum number of obstacles */
    int obstacles_max = cd_pyth(src,dst) *3;
    /* allocate the list of blocked angle_sets. */
    struct angle_set blocked_list[obstacles_max];

    return visible;
}

bool chl_in_radius(struct chl_fov_set *set, coord_t *src, coord_t *dst, int radius) {
    if (set == NULL) return false;
    if (cd_within_bound(src, &set->size) == false) return false;
    if (cd_within_bound(dst, &set->size) == false) return false;

    /* find out which octant we are in */
    enum chl_octant octant = get_octant(src, dst);
    /* get the octant describtor */
    struct chl_octant_quad *oct_mod = &octant_lo_table[octant];

    /* find the row and cell nr of the destination, with regard to the octant we are in. */
    coord_t delta = cd_delta_abs(src, dst);
    int cell_dst = delta.x;
    int row_dst = delta.y;
    if (oct_mod->flip) {
        cell_dst = delta.y;
        row_dst = delta.x;
    }

    /* check radius */
    return in_radius(set, row_dst, cell_dst, radius);
}

