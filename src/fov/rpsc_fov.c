#include <limits.h>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>

//#define RPSC_DEBUG

#include "rpsc_fov.h"

#ifdef RPSC_DEBUG
#include "logging.h"
#else
#define lg_debug(a, ...) do { } while (0);
#endif

/* 
   This is an implementation of:
    http://www.roguebasin.com/index.php?title=Restrictive_Precise_Angle_Shadowcasting

    Short overview:
    We divide the circle in 8 octants. Each octant having a row and a line of cells.
    the number of cells in a line is the row number +1. We start at the first row, 
    since row 0, cell 0 is the origin that is neither a blocker nor blocked.

    For every cell in the row, we calculate angles. We have a angle range, 
    the article uses 1.0f, but I use a large integer number; doesn't really matter.
    The angle range is divided over the cells which divide it over three angles per cell,
    near, center and far, with [cell 0].far being equal to [cell 1].near.

    Per row, we first check if an cells angles are blocked by a blocking cell from 
    a previous row. We do not need to check to current rows, because a cell on the 
    current row can never block a cell on that row. 
    If it is blocked, we add it to a blocking list and it will be used on the next row.
    If it is not, we check if it itself can block others (i.e. not opaque) and if so, 
    add it to the blocking list and it will be used on the next row.

    At the end of a row, we try to combine all the blocking cells into one big blocker.
    We do this for 3 reasons: 
        1) It is easy to check if we are done, if we have one cell blocking the whole 
           octant there is no need to continue.
        2) Speed, having less entries in the blocking list saves us looping through 
           more entries.
        3) Size, we can have a fairly small list. Idealy the list should be the 
           size of the number of cells in the last row, but because we combine them 
           only after the row, it should be a little bit bigger.

    TODO:
    - fix cone fov, for area of effect weapons like flamers.

    IDEAS:
    - Right now, we hold that [cell 0].far == [cell 1].near. Maybe it would be 
      easier to make them distinct.
    - Maybe we can increase the number of angles per cell, for example, to divide 
      a cell into quadrants and calculate per quadrant if it is blocking. Then we
      can set the strictness to the number of quadrants which should be blocked 
      before a cell is really blocked.
*/

/* IDs of all octants */
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

/* describtion of each octant */
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

/*  
    instead of using floating point, we use a 16bit integer (and even more when available)
*/
typedef uint_fast16_t angle_t;

#ifdef RPSC_DEBUG
#define FP_MAX UINT16_MAX /* use this when debugging to avoid becoming number crazy */
#else
#define FP_MAX UINT_FAST16_MAX
#endif

#define FP_HALF ( (FP_MAX>>1) & ~0xF)
#define ANGLE_RANGE FP_HALF

/* this describes the angles */
struct angle_set {
    angle_t near;
    angle_t center;
    angle_t far;
};

/* searches the octant table and retrieves the corresponding octant */
enum rpsc_octant get_octant(coord_t *src, coord_t *dst) {
    coord_t delta = cd_delta_abs(src, dst);

    /* loop through the octant array, in search for the one which describes our situation */
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

/* calculate the angles for this cell */
static inline struct angle_set offset_to_angle_set(int row, int cell) {
    struct angle_set set;
    angle_t max_range = ANGLE_RANGE;

    /* range per cell */
    angle_t range = (max_range / (row +1) );

    /* starting angle */
    set.near = range * cell;

    /* far angle is equal to the starting angle of the next cell */
    set.far  = set.near + range;

    /* center angle is in between */
    set.center = set.near + (range / 2);

    lg_debug("as[%d,%d], (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ")", row,cell, set.near,set.center,set.far);

    /* some paranoia checks */
    assert(set.near < set.center);
    assert(set.center < set.far);

    /* return calculated set */
    return set;
}

/* convert a given center angle to a cell in the given row */
static inline int angle_set_to_cell(struct angle_set *set, int row_new) {
    /* get the range within this row */
    angle_t new_range = ANGLE_RANGE / (row_new +1);

    /* get the cell number within this row. this can be on near 
       the border of this cell, but we'll handle that later */
    return set->center / new_range;
}

/* check if a given angle set falls within the blocked set, given certain parameters */
inline static bool angle_is_blocked(struct rpsc_fov_set *set, struct angle_set *test_set, struct angle_set *blocked_set) {

    /* if it falls completely outside the blocked set, our job is done */
    if (test_set->far < blocked_set->near) return false;
    if (test_set->near > blocked_set->far) return false;

    /* three bools for three angles */
    bool near_blocked = false;
    bool center_blocked = false;
    bool far_blocked = false;

    /* least restrictive */
    if (set->visible_on_equal) {
        /* check if the near angle falls within between the blocked angle set*/
        if ( (test_set->near   > blocked_set->near) && (test_set->near   < blocked_set->far) ) near_blocked = true;
        /* check center angle */
        if ( (test_set->center > blocked_set->near) && (test_set->center < blocked_set->far) ) center_blocked = true;
        /* check far angle */
        if ( (test_set->far    > blocked_set->near) && (test_set->far    < blocked_set->far) ) far_blocked = true;
    }
    else {
        /* check if the near angle falls within between the blocked angle set*/
        if ( (test_set->near   >= blocked_set->near) && (test_set->near   <= blocked_set->far) ) near_blocked = true;
        /* check center angle */
        if ( (test_set->center >= blocked_set->near) && (test_set->center <= blocked_set->far) ) center_blocked = true;
        /* check far angle */
        if ( (test_set->far    >= blocked_set->near) && (test_set->far    <= blocked_set->far) ) far_blocked = true;
    }

    /* given the permissiveness, return whether or not the test_set is blocked */
    if (set->permissiveness == RPSC_FOV_PERMISSIVE_RELAXED) {
        /* only blocked when all angles are blocked */
        return (near_blocked && center_blocked && far_blocked);
    }
    else if (set->permissiveness == RPSC_FOV_PERMISSIVE_STRICT) {
        /* blocked when a angle is blocked */
        return (near_blocked || center_blocked || far_blocked);
    }

    /* blocked when any two angles are blocked */
    return (near_blocked && center_blocked) || (center_blocked && far_blocked);
}

/* check if (row,cell) is within radius with the given settings */
inline static bool in_radius(struct rpsc_fov_set *set, int row, int cell, int radius) {
    /* if visible_on_equal == false, we are more strict */
    if (set->visible_on_equal == false) radius -= 1;

    if (set->area == RPSC_AREA_OCTAGON) {
        if ( ( ( (row) + (cell/2) ) ) <= radius) return true;
    }
    else if (set->area == RPSC_AREA_CIRCLE) {
        /* nicer circle */
        if ( ( (row*row) + (cell*cell) ) <= ((radius*radius) + radius) ) return true;
    }
    else if (set->area == RPSC_AREA_CIRCLE_STRICT) {
        /* pythagoras circle, which does not translate well to an ascii grid. */
        if ( ( (row*row) + (cell*cell) ) <= (radius*radius) ) return true;
    }
    else if (set->area == RPSC_AREA_SQUARE) {
        /* simple square, row is always bigger than the cell. */
        if (row <= radius) return true;
    }
    return false;
}

/* extend a blocked_set with another. expects the current_set to border with blocked_set */
inline static bool extend_block(struct angle_set *blocked_set, struct angle_set *current_set) {
    /* extend block */
    if (current_set->near < blocked_set->near) blocked_set->near = current_set->near;
    if (current_set->far > blocked_set->far) blocked_set->far = current_set->far;

    /* paranoia */
    assert(blocked_set->near < blocked_set->center);
    assert(blocked_set->center < blocked_set->far);
    return true;
}

/* check the list of blocking sets to see if we can combine them */
static int scrub_blocked_list(struct angle_set *list, int list_sz) {

    /* this holds the largest unique index,
    i.e. one which was not combined with another */
    int max_not_scrubbed = -1;

    lg_debug("scrub start, list sz is %d", list_sz);

    /* loop the list from newest to oldest */
    for (int i = list_sz -1; i >= 0; i--) {
        struct angle_set *a = &list[i];
        bool combined = false;

        /* compare them with existing members, from oldest to newest */
        for (int j = 0; (j < list_sz -1) && (combined == false); j++) {

            /* ignore myself */
            if (i == j) continue;

            /* if we border atleast one region */
            if ( ( (a->near >= list[j].near) && (a->near <= list[j].far) ) ||
                 ( (a->far <= list[j].far)   && (a->far >= list[j].near) ) ) {

                /* combine them */
                extend_block(&list[j], a);

                /* shrink the list */
                list_sz--;
                /* we were succesfull */
                combined = true;
                lg_debug("combining [%d] (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ") with [%d] (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ")", i, a->near, a->center, a->far, j, list[j].near, list[j].center, list[j].far);
            }
        }

        /* if we did not combine, and if max_not_scrubbed was not set by another,
           the returned list must be atleast the size of this index */
        if ( (combined == false) && (max_not_scrubbed < i) ) max_not_scrubbed = i+1;
    }

    lg_debug("scrub end, list sz is %d", MAX(list_sz, max_not_scrubbed));

    /* 
       Ok, we combined some new entries with existing ones.
       problem is, that it is possible to have an non-continues list,
       [3] is not combined, while [2] and [1] were combined with [0].
       We can check all entries and move [3] to [1], but that requires 
       atleast another loop, probably 2.

       What we do now is simple return 4 as the list size, and try 
       next time if we can combine [3] with [0].
     */
    return MAX(list_sz, max_not_scrubbed);
}

/* for every cell within this octant, check if we can see it. */
static void rpsc_fov_octant(struct rpsc_fov_set *set, coord_t *src, int radius, enum rpsc_octant octant, angle_t min_angle, angle_t max_angle) {

    /* list of blocking cells. We should ideally only need <radius> nr of items, but since we 
       are combining them only after a row, we need some leeway. */
    struct angle_set blocked_list[radius *2];

    /* number of obstacles up untill the previous line*/
    int obstacles_total = 0;

    /* descriptor of this octant */
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];

    lg_debug("-------------start in octand %s----------", oct_mod->desc);

    /* set boundries */
    if (min_angle > 0) {
        /* we have a minimum angle, so we'll just add an obstacle obscuring the rest. */
        struct angle_set a = { .near = 0, .center = min_angle /2, .far = min_angle, };
        blocked_list[obstacles_total++] = a;
    }
    if (max_angle < ANGLE_RANGE) {
        /* we have a maximum angle, so we'll just add an obstacle obscuring the rest. */
        struct angle_set a = { .near = max_angle, .center = max_angle + (ANGLE_RANGE - max_angle) /2, .far = ANGLE_RANGE, };
        blocked_list[obstacles_total++] = a;
    }

    /* for every row intill radius. row 0, cell 0 is the origin. */
    for (int row = 1; row <= radius; row++) {
        /* number of obstacles found this row, these will start to block cells from the next row. */
        int obstacles_this_row = 0;
        /* number of cells in this row */
        int row_max = row+1;

        /* 
           At the end we check if the octant is blocked by checking 
           if there is a unifying obstacle. However there can be rounding 
           issues, therefor we keep track of the maximum angle used this row 
           and use that to check if the complete row is blocked.
         */
        angle_t angle_min = 0;
        angle_t angle_max = ANGLE_RANGE;

        /* for every cell in this row */
        for (int cell = 0; cell < row_max; cell++) {

            /* create an absolute point, using the info from the octant */
            coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
            if (oct_mod->flip) {
                point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
            }

            lg_debug("row %d, cell %d, point (%d,%d)", row, cell, point.x,point.y);

            /* check if the point is on the map */
            if (cd_within_bound(&point, &set->size) ) {
                bool blocked = false;

                /* calculated the angles of this cell */
                struct angle_set as = offset_to_angle_set(row, cell);

                /* set the minimum and maximum angles encountered */
                if (cell == 0) angle_min = as.near;
                if (cell == row) angle_max = as.far;

                /* loop through the blocked list and check if the current cell is blocked by any of them. */
                for (int i = 0; (i < obstacles_total) && (blocked == false); i++) {
                    lg_debug("test (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ") vs [%d] (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ")", as.near, as.center, as.far, i, blocked_list[i].near, blocked_list[i].center, blocked_list[i].far);

                    /* check if <as> if blocked or outside the view area. */
                    if (angle_is_blocked(set, &as, &blocked_list[i]) || 
                       in_radius(set, row, cell, radius) == false) {

                        lg_debug("blocked by [%d]", i);

                        /* if it is, and if the setting <not_visible_blocks_vision>, is true, add it to the blocklist */
                        if (set->not_visible_blocks_vision) {
                            blocked_list[obstacles_total + obstacles_this_row] = as;
                            obstacles_this_row++;
                            lg_debug("becomes obstacle [%d]", obstacles_this_row + obstacles_total -1);
                        }
                        else lg_debug("but does not block vision");

                        /* this cell is check and found blocking */
                        blocked = true;
                    }
                }

                /* the cell is visible, now check if it will block others. */
                if (blocked == false) {
                    /* inform the user. */
                    if (set->apply != NULL) set->apply(set, &point, src);

                    /* check if it will block others */
                    if (set->is_opaque(set, &point, src) == false) {
                        /* it does, adding it to the blocklist */
                        blocked_list[obstacles_total + obstacles_this_row] = as;
                        obstacles_this_row++;
                        lg_debug("visible obstacle [%d]", obstacles_this_row + obstacles_total -1);
                    }
                    else lg_debug("visible floor");
                }
            }
        }
    
        if (obstacles_this_row > 0)  {
            /* we have gathered some new blockers, lets cleanup the list. */
            obstacles_total = scrub_blocked_list(blocked_list, obstacles_total + obstacles_this_row);

            /* if there is one obstacle, which covers the minimum and maximum angle 
               used in this row, it blocks everything. Thus there is no need to continue. */
            if ( (obstacles_total == 1) && (blocked_list[0].near <= angle_min) && (blocked_list[0].far >= angle_max) ) {
                lg_debug("done: %" PRIuFAST16 ",%" PRIuFAST16 "", blocked_list[0].near, blocked_list[0].far);
                return;
            }
        }
    }

    return;
}

/* calculates the complete fov */
bool rpsc_fov(struct rpsc_fov_set *set, coord_t *src, int radius) {
    if (set == NULL) return false;
    if (set->is_opaque == NULL) return false;
    if (cd_within_bound(src, &set->size) == false) return false;

    if (set->apply != NULL) set->apply(set, src, src);
    lg_debug("fov start src: (%d,%d)", src->x,src->y);

    /* loop through the octants and calculate each octant. */
    for (int i = 0; i < OCTANT_MAX; i++) {
        rpsc_fov_octant(set,src,radius, i, 0, ANGLE_RANGE);
    }
    return true;
}

/* TBI */
bool rpsc_cone(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, int angle, int radius) {
    if (set == NULL) return false;
    if (set->is_opaque == NULL) return false;
    if (cd_within_bound(src, &set->size) == false) return false;
    if (cd_within_bound(dst, &set->size) == false) return false;

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

    return true;
}

/* calculate the los from one point to another.
   In essence we use the exact same algorithm as rpsc_octant,
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
bool rpsc_los(struct rpsc_fov_set *set, coord_t *src, coord_t *dst) {
    if (set == NULL) return false;
    if (set->is_opaque == NULL) return false;
    if (cd_within_bound(src, &set->size) == false) return false;
    if (cd_within_bound(dst, &set->size) == false) return false;
    bool visible = true;

    /* find out which octant we are in */
    enum rpsc_octant octant = get_octant(src, dst);
    /* get the octant describtor */
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];

    /* allocate the list of blocked angle_sets. */
    struct angle_set blocked_list[cd_pyth(src,dst) *2];
    /* total number of obstacles found up untill the previous line */
    int obstacles_total = 0;

    if (set->apply != NULL) lg_debug("-------------with apply start in octand %s----------", oct_mod->desc);
    if (set->apply != NULL) set->apply(set, src, src);

    /* find the row and cell nr of the destination, with regard to the octant we are in. */
    coord_t delta = cd_delta_abs(src, dst);
    int cell_dst = delta.x;
    int row_dst = delta.y;
    if (oct_mod->flip) {
        cell_dst = delta.y;
        row_dst = delta.x;
    }

    /* find the angles of the destination  */
    struct angle_set as_dst = offset_to_angle_set(row_dst, cell_dst);
    lg_debug("los: (%d,%d) -> (%d,%d), length: %d", src->x,src->y,dst->x,dst->y, row_dst);

    /* loop through the rows untill the desination row, or when we have 
       determined that the destination cannot be visible. 

       We will create a line of three cells thick from src to dst, adding 
       obstacles as we go. Ideally we take the center cell as our path, 
       but if that is blocked we take one of the other cells. 
     */
    for (int row = 1; (row <= row_dst) && (visible == true); row++) {

        /* the center_cell is the ideal line towards destination */
        int center_cell = angle_set_to_cell(&as_dst, row);

        /* modifiers for the three cells, our preference is the center cell, but we also 
           take the others into account. */
        int cell_select[3] = {0,-1,1};

        /* describes if this row allready has cell reported to the user. */
        bool applied = false;

        /* number of obstacles added this row. */
        int obstacles_this_row = 0;

        /* loop throught the 3 cells of this row */
        for (int c = 0; c <= 2; c++) {
            bool blocked = false;
            bool destination = false;

            /* get the cell we are interrested in. */
            int cell = center_cell +cell_select[c];
            lg_debug(">>> row %d, cell %d, c: %d", row, cell, cell_select[c]);

            /* bounds check */
            if (cell < 0) continue;
            if (cell > row) continue;

            /* exceptional case:
               we are now in the destination row, the other 2 cells are now unable 
               to block the destination, and neither are they relevant to our path 
               anymore. The destination cell itself however could still be blocked 
               by the previous row. this we do calculate this row, but only for the 
               destination cell itsef. */
            if (cell_select[c] != 0 && row == row_dst) continue;

            /* get the absolute point this cell represents. */
            coord_t point = cd_create(src->x + (cell * oct_mod->x), src->y + (row * oct_mod->y));
            if (oct_mod->flip) {
                point = cd_create(src->x + (row * oct_mod->x), src->y + (cell * oct_mod->y));
            }
            lg_debug("next point is (%d,%d)", point.x,point.y);

            if (cd_equal(&point,dst) ) destination = true;

            /* the angles for this cell. */
            struct angle_set as = offset_to_angle_set(row, cell);

            /* loop through the obstacles and check if this cell if being blocked. */
            for (int i = 0; i < obstacles_total; i++) {
                lg_debug("test (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ") vs [%d] (%" PRIuFAST16 ",%" PRIuFAST16 ",%" PRIuFAST16 ")", as.near, as.center, as.far, i, blocked_list[i].near, blocked_list[i].center, blocked_list[i].far);

                /* check if the angles of this cell border a blocked cell or if it is outside the view area. */
                if (angle_is_blocked(set, &as, &blocked_list[i]) ) {

                    lg_debug("blocked by [%d]", i);

                    /* we are blocked, if <not_visible_blocks_vision> is set, we
                       block the vision of other cells. */
                    if (set->not_visible_blocks_vision) {
                        /* it is, adding this cell to the block list. */
                        blocked_list[obstacles_total + obstacles_this_row] = as;
                        obstacles_this_row++;
                        lg_debug("becomes obstacle [%d]", obstacles_this_row + obstacles_total -1);
                    }
                    else lg_debug("but does not block vision");

                    /* this cell is blocked. */
                    blocked = true;
                }
            }

            /* the cell is not blocked by another. now we check it will block others. */
            if (blocked == false) {
                /* 
                   We check if this cell will block others (i.e. is a wall).
                   Given a strict is_opaque funciton however, the target square 
                   could also be blocked (for example to check line of fire, but the 
                   target square also has an actor).

                   Thus we do not do block the target square.
                 */
                if ( (set->is_opaque(set, &point, src) == false) && (destination == false) ) {
                    /* add it to the obstacle list. */
                    blocked_list[obstacles_total + obstacles_this_row] = as;
                    obstacles_this_row++;
                    lg_debug("becomes obstacle [%d]", obstacles_this_row + obstacles_total -1);

                    /* and set blocking to true. */
                    blocked = true;
                }
                else if (set->apply != NULL && applied == false) {
                    /* if we have not yet applied in this row, 
                       and it is not blocked, apply this cell. 
                     This works because we are looping through 
                     the cells on priority.*/
                    set->apply(set, &point, src);
                    applied = true;
                }
            }

            if ( (blocked) && (destination) ) {
                /* if this cell is the destination, than it is 
                   a done deal if it itself is blocked. */
                visible = false;
            }
        }

        if (obstacles_this_row > 0 && visible == true)  {
            /* we have gathered some new blockers, lets cleanup the list. */
            obstacles_total = scrub_blocked_list(blocked_list, obstacles_total + obstacles_this_row);

            /* if there is one obstacle, which covers the minimum and maximum angle 
               used in this row, it blocks everything. Thus there is no need to continue. */
            if ( (obstacles_total == 1) && (blocked_list[0].near <= as_dst.near) && (blocked_list[0].far >= as_dst.far) ) {
                visible = false;
            }
        }
    }

    if (set->apply != NULL) lg_debug("-------------with apply end, visibility is %s----------", (visible) ? "true": "false");
    return visible;
}

bool rpsc_in_radius(struct rpsc_fov_set *set, coord_t *src, coord_t *dst, int radius) {
    if (set == NULL) return false;
    if (cd_within_bound(src, &set->size) == false) return false;
    if (cd_within_bound(dst, &set->size) == false) return false;

    /* find out which octant we are in */
    enum rpsc_octant octant = get_octant(src, dst);
    /* get the octant describtor */
    struct rpsc_octant_quad *oct_mod = &octant_lo_table[octant];

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

