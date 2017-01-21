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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "dungeon_bsp.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "logging.h"
#include "coord.h"
#include "random.h"
#include "ai/pathfinding.h"

enum AXIS {
    X,
    Y,
    AXIS_MAX,
};

struct fallen_wall {
    coord_t a;
    coord_t b;
};

#define NEIGHBOUR_MAX 12
#define NEIGHBOUR_LEN_MIN 10
struct bsp_area {
    coord_t size;
    coord_t ul;
    int level;
    bool split;
    struct bsp_area *neighbours[NEIGHBOUR_MAX];
    struct bsp_area *partner;
    int nr_nghbrs;
    struct fallen_wall wall;
    bool route;
    bool connected;
    int extra_cost;
    coord_t entry;
};
/*
static int area(coord_t *a) {
    return a->x * a->y;
}
*/
// at least 2 cells shared by both areas
static bool test_lines(int p1, int p2, int s1, int s2) {
    if ( ((s1 >= p1) && (p2-s1>2)) || ((s1 <= p1) && (s2-p1>2))) {
        return true;
    }
    return false;
}

static bool is_point_in_area(struct bsp_area *a, coord_t *b) {
    if (a->ul.x > b->x || a->ul.y > b->y) return false;
    if (a->ul.x + a->size.x < b->x || a->ul.y + a->size.y < b->y) return false;
    return true;
}

static void spread_ip_strength(struct bsp_area *a, int str) {
    if (str <= 0) return;

    lg_debug("\tspreaing influence: %d (%d,%d,%d,%d)", str, a->ul.x, a->ul.y, a->ul.x + a->size.x, a->ul.y + a->size.y);
    if (a->extra_cost == 0) {
        a->extra_cost += str;
        lg_debug("\t\tneighbour extra cost: %d (%d,%d,%d,%d)", a->extra_cost, a->ul.x, a->ul.y, a->ul.x + a->size.x, a->ul.y + a->size.y);
    }
    for (int i = 0; i < a->nr_nghbrs; i++) {
        spread_ip_strength(a->neighbours[i], str - 1);
    }
}

static void add_nghbr(struct bsp_area *a, struct bsp_area *b) {
    if (a->nr_nghbrs < NEIGHBOUR_MAX) {
        a->neighbours[a->nr_nghbrs] = b;
        a->nr_nghbrs++;
    }
    if (b->nr_nghbrs < NEIGHBOUR_MAX) {
        b->neighbours[b->nr_nghbrs] = a;
        b->nr_nghbrs++;
    }
    lg_debug("\tnew neighbour: (%d,%d,%d,%d)", b->ul.x, b->ul.y, b->ul.x + b->size.x, b->ul.y + b->size.y);
}

static int point_to_area_idx(struct bsp_area *list, int list_len, coord_t *c, int skip) {
    if (skip < 0) skip = 0;
    for (int i = skip; i < list_len; i++) {
        struct bsp_area *bs = &list[i];
        //if (bs->split == true) continue;
        if (is_point_in_area(bs, c) == true) return i;
    }
    lg_debug("could not find (%d,%d) in area list with offset %d(/%d)", c->x, c->y, skip, list_len);
    return -1;
}

static bool point_in_wall(struct bsp_area *a, coord_t *c) {
    if (is_point_in_area(a, c) == false) return false;
    if ( (c->x == a->ul.x) ||
         (c->y == a->ul.y) ||
         (c->x == a->ul.x + a->size.x) ||
         (c->y == a->ul.y + a->size.y) ) {
        return true;
    }
    return false;
}

static coord_t nghbr_entry(struct bsp_area *a, struct bsp_area *b, struct random *r) {
    coord_t entry = cd_create(0,0);

    if (a->ul.x == b->ul.x + b->size.x) {
        int range = a->size.y -3;
        int mod = (random_int32(r) % range) +2;
        entry = a->ul;
        entry.y += mod;
    }
    else if (a->ul.y == b->ul.y + b->size.y) {
        int range = a->size.x -3;
        int mod = (random_int32(r) % range) +2;
        entry = a->ul;
        entry.x += mod;
    }
    else if (a->ul.x + a->size.x == b->ul.x) {
        int range = a->size.y -3;
        int mod = (random_int32(r) % range) +2;
        entry = a->ul;
        entry.x += a->size.x;
        entry.y += mod;
    }
    else if (a->ul.y + a->size.y == b->ul.y) {
        int range = a->size.x -3;
        int mod = (random_int32(r) % range) +2;
        entry.y += a->size.y;
        entry.x += mod;
    }
    return entry;
}

struct pf_cb_struct {
    int sz;
    struct bsp_area *list;
};

static unsigned int pf_callback(void *vmap, coord_t *coord) {
    if (vmap == NULL) return PF_BLOCKED;
    if (coord == NULL) return PF_BLOCKED;

    int cost = 1;

    struct pf_cb_struct *ctx = vmap;
    for (int i = 0; i < ctx->sz; i++) {
        if (ctx->list[i].split == true) continue;
        if (is_point_in_area(&ctx->list[i], coord) == false) continue;

        if (ctx->list[i].extra_cost > 0) {
            cost += (ctx->list[i].extra_cost * 5);
        }

        /* if in wall */
        if (point_in_wall(&ctx->list[i], coord) ) {
            cost += 10;
        }
    }

    return cost;
}

bool dm_generate_map_bsp(struct dm_map *map, struct random *random, coord_t *ul, coord_t *dr) {
    int max_div_levels = 7 + (random_int32(random) %5);
    unsigned int alist_sz = 1;
    struct bsp_area area_list[ (max_div_levels * max_div_levels) +1];
    memset(area_list, 0x0, sizeof(area_list));

    coord_t size = { .x = dr->x - ul->x, .y = dr->y - ul->y, };

    area_list[0].size = size;
    area_list[0].ul = *ul;
    area_list[0].level = 0;
    area_list[0].split = false;
    area_list[0].nr_nghbrs = 0;
    area_list[0].partner = NULL;
    area_list[0].extra_cost = 0;
    area_list[0].route = false;
    area_list[0].connected = false;
    area_list[0].wall.a = cd_create(0,0);
    area_list[0].wall.b = cd_create(0,0);
    area_list[0].entry = cd_create(0,0);

    for (int l = 0; l < max_div_levels; l++) {
        int incr = 0;
        for (unsigned int a = 0; a < alist_sz && a < ARRAY_SZ(area_list); a++) {
            if (alist_sz +incr +2 >= ARRAY_SZ(area_list) ) break;

            struct bsp_area *ba = &area_list[a];
            if (ba->split == true) continue;
            //if (ba->size.x == 0) continue;
            lg_debug("splitting area[%d: l %d]: (sz %d,%d) (ul %d,%d) lvl: %d", a, l, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

            /* Choose division axis */
            int axis_r = random_int32(random) % AXIS_MAX;
            int *axis = (axis_r == X) ? &ba->size.x : &ba->size.y;
            lg_debug("\taxis check area[%d, a %d]: (sz %d)", a, axis_r, *axis);
            if (*axis <= (2 * NEIGHBOUR_LEN_MIN) ) {
                axis_r = (axis_r == X) ? Y : X;
                axis = (axis_r == X) ? &ba->size.x : &ba->size.y;
                lg_debug("\taxis check area[%d, a %d]: (sz %d)", a, axis_r, *axis);

                if (*axis <= (2 * NEIGHBOUR_LEN_MIN) ) {
                    lg_debug("\t--> error: too small");
                    continue;
                }
            }
            int range = *axis - (2 * NEIGHBOUR_LEN_MIN);
            int div_range = (random_int32(random) % range) + NEIGHBOUR_LEN_MIN;
            lg_debug("\t\taxis[%d -> %d] div_range %d", axis_r, *axis, div_range);

            /* Add both areas to area list */
            struct bsp_area *ba1 = &area_list[alist_sz +incr];
            ba1->size = cd_create( (axis_r == X) ? div_range : ba->size.x,
                                   (axis_r == Y) ? div_range : ba->size.y);
            ba1->ul   = cd_create(ba->ul.x, ba->ul.y);
            ba1->level = l +1;
            ba1->split = false;
            ba1->nr_nghbrs = 0;
            ba1->extra_cost = 0;
            ba1->route = false;
            ba1->partner = NULL;
            ba1->connected = false;
            ba1->wall.a = cd_create(0,0);
            ba1->wall.b = cd_create(0,0);
            ba1->entry = cd_create(0,0);
            lg_debug("\t\tarea[%u]: (sz %d,%d) (ul %d,%d) lvl: %d", alist_sz +incr, ba1->size.x, ba1->size.y, ba1->ul.x, ba1->ul.y, ba1->level);

            struct bsp_area *ba2 = &area_list[alist_sz +incr +1];
            ba2->size = cd_create( (axis_r == X) ? ba->size.x - div_range : ba->size.x,
                                   (axis_r == Y) ? ba->size.y - div_range : ba->size.y);
            ba2->ul   = cd_create( (axis_r == X) ? (ba->ul.x + div_range) : ba->ul.x,
                                   (axis_r == Y) ? (ba->ul.y + div_range) : ba->ul.y );
            ba2->level = l +1;
            ba2->split = false;
            ba2->nr_nghbrs = 0;
            ba2->extra_cost = 0;
            ba2->route = false;
            ba2->connected = false;
            ba2->partner = NULL;
            ba2->wall.a = cd_create(0,0);
            ba2->wall.b = cd_create(0,0);
            ba2->entry = cd_create(0,0);
            lg_debug("\t\tarea[%u]: (sz %d,%d) (ul %d,%d) lvl: %d", alist_sz +incr +1, ba2->size.x, ba2->size.y, ba2->ul.x, ba2->ul.y, ba2->level);

            ba1->partner = ba2;
            ba2->partner = ba1;

            /* Increase list */
            ba->split = true;
            incr += 2;
        }
        alist_sz += incr;

        /* create neighbour lists */
        for (unsigned int a = 1; a < alist_sz; a++) {
            struct bsp_area *bs_a = &area_list[a];
            if (bs_a->level != l + 1) continue;

            lg_debug("adding neighbours to: (%d,%d,%d,%d)", bs_a->ul.x, bs_a->ul.y, bs_a->ul.x + bs_a->size.x, bs_a->ul.y + bs_a->size.y);
            for (unsigned int b = 1; b < alist_sz; b++) {
                if (a == b) continue;
                struct bsp_area *bs_b = &area_list[b];
                if (bs_b->level != l + 1) continue;

                bool found = false;
                if (bs_a->ul.x == bs_b->ul.x + bs_b->size.x) {
                    lg_debug("\ttest_lines 1: vs (%d,%d,%d,%d)",
                            bs_b->ul.x, bs_b->ul.y, bs_b->ul.x + bs_b->size.x, bs_b->ul.y + bs_b->size.y);
                    found |= test_lines(bs_a->ul.y, bs_a->ul.y + bs_a->size.y, bs_b->ul.y, bs_b->ul.y + bs_b->size.y);
                }
                else if (bs_a->ul.x + bs_a->size.x == bs_b->ul.x) {
                    lg_debug("\ttest_lines 2: vs (%d,%d,%d,%d)",
                            bs_b->ul.x, bs_b->ul.y, bs_b->ul.x + bs_b->size.x, bs_b->ul.y + bs_b->size.y);
                    found |= test_lines(bs_a->ul.y, bs_a->ul.y + bs_a->size.y, bs_b->ul.y, bs_b->ul.y + bs_b->size.y);
                }
                else if (bs_a->ul.y == bs_b->ul.y +bs_b->size.y) {
                    lg_debug("\ttest_lines 3: vs (%d,%d,%d,%d)",
                            bs_b->ul.x, bs_b->ul.y, bs_b->ul.x + bs_b->size.x, bs_b->ul.y + bs_b->size.y);
                    found |= test_lines(bs_a->ul.x, bs_a->ul.x + bs_a->size.x, bs_b->ul.x, bs_b->ul.x + bs_b->size.y);
                }
                else if (bs_a->ul.y +bs_a->size.y == bs_b->ul.y) {
                    lg_debug("\ttest_lines 4: vs (%d,%d,%d,%d)",
                            bs_b->ul.x, bs_b->ul.y, bs_b->ul.x + bs_b->size.x, bs_b->ul.y + bs_b->size.y);
                    found |= test_lines(bs_a->ul.x, bs_a->ul.x + bs_a->size.x, bs_b->ul.x, bs_b->ul.x + bs_b->size.x);
                }

                if (found) {
                    add_nghbr(bs_a, bs_b);
                }
            }
        }
    }

    /* calculate influence */
    int nr_influence_points = random_int32(random) % 2 +3;
    coord_t ip_list[nr_influence_points];
    for (int i = 0; i < nr_influence_points; i++) {
        /* place influence points */
        bool placed = false;
        for (unsigned int j = 0; j < alist_sz && placed == false; j++) {
            coord_t ip = cd_create( (random_int32(random) % (size.x - 6) ) + 3,
                                    (random_int32(random) % (size.y - 6) ) + 3);
            int strength = (random_int32(random) % 3) +2;
            if (i == 0) {
                ip = cd_create( (random_int32(random) % (size.x / 5) ) + ( (size.x / 2) - size.x / 5 ),
                                        (random_int32(random) % (size.y / 5) ) + ( (size.y / 2) - size.y / 5 ) );
                strength = 3;
            }

            /*
            * find area which contains the influence,
            *  for every neighbour, add cost
            *    for ever neighbour neighbour add cost -1
            *      ...
            */
            int aidx = point_to_area_idx(area_list, alist_sz, &ip, j);
            if (aidx < 0) continue;
            struct bsp_area *bs = &area_list[aidx];
            if (bs == NULL) continue;
            if (bs->split == true) continue;
            lg_debug("neighbour ip: %d (%d,%d,%d,%d)", strength, bs->ul.x, bs->ul.y, bs->ul.x + bs->size.x, bs->ul.y + bs->size.y);
            spread_ip_strength(bs, strength);
            ip_list[i] = ip;
            placed = true;
        }
        assert(placed);
    }

    /* pathfinder */
    /*
     * - Passing through walls cost extra
     * - take influence of room into accout
     * - play route, and note which walls not to place.
     */
    struct pf_cb_struct pf_cb_s = {
        .sz   = alist_sz,
        .list = area_list,
    };

    struct pf_context *pf_ctx = NULL;
    struct pf_settings pf_set = {
        .map_start = {
            .x = 2,
            .y = 2,
        },
        .map_end = {
            .x = map->sett.size.x -2,
            .y = map->sett.size.y -2,
        },
        .map = &pf_cb_s,

        .nhlo_tbl   = coord_nhlo_table_orth,
        .nhlo_tbl_sz = coord_nhlo_table_orth_sz,

        .pf_traversable_callback = pf_callback,
    };

    coord_t start = cd_create( 5,5);
    coord_t end   = cd_create(map->sett.size.x -5, map->sett.size.y - 5);
    assert( (pf_ctx = pf_init(&pf_set) ) != NULL);

    /* Run pathfinder */
    assert (pf_astar_map(pf_ctx, &start, &end) );
    coord_t *coord_lst = NULL;

    /* We got a possible, let's fetch it */
    int pf_len = pf_calculate_path(pf_ctx, &start, &end, &coord_lst);
    assert (pf_len > 0);

    /* And play it here, indicating to the area to ignore a certain wall. */
    for (int i = 0; i < pf_len; i++) {
        coord_t *c = &coord_lst[i];
        int j = 0;
        while ( (j = point_to_area_idx(area_list, alist_sz, c, j+1) ) >= 0) {
            struct bsp_area *bsp = &area_list[j];
            bsp->connected = true;
            if (bsp->split == true) continue;
            bsp->route = true;

            /* tell area to ignore this wall */
            if (point_in_wall(bsp, c) ) {
                /* find out which wall */
                if (c->x == bsp->ul.x) {
                    bsp->wall.a = bsp->ul;
                    bsp->wall.b = cd_create(bsp->ul.x, bsp->ul.y + bsp->size.y);
                }
                else if (c->y == bsp->ul.y) {
                    bsp->wall.a = bsp->ul;
                    bsp->wall.b = cd_create(bsp->ul.x + bsp->size.x, bsp->ul.y);
                }
                else if (c->x == bsp->ul.x + bsp->size.x) {
                    bsp->wall.a = cd_create(bsp->ul.x + bsp->size.x, bsp->ul.y);
                    bsp->wall.b = cd_create(bsp->ul.x + bsp->size.x, bsp->ul.y + bsp->size.y);
                }
                else if (c->y == bsp->ul.y + bsp->size.y) {
                    bsp->wall.a = cd_create(bsp->ul.x, bsp->ul.y + bsp->size.y);
                    bsp->wall.b = cd_create(bsp->ul.x + bsp->size.x, bsp->ul.y + bsp->size.y);
                }
            }
        }
    }
    for (unsigned int a = 1; a < alist_sz; a++) {
        struct bsp_area *b = &area_list[a];
        if (b->connected == true) continue;

        struct bsp_area *p = b->partner;
        coord_t entry = nghbr_entry(b,p, random);
        if (p->connected) b->connected = true;
        p->entry = b->entry = entry;
    }

    /* paint areas */
    for (unsigned int a = 1; a < alist_sz; a++) {
        struct bsp_area *ba = &area_list[a];
        if (ba->split == true) continue;
        if (ba->route == false) continue;
        lg_debug("painting area[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", a, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

        coord_t c = cd_create(0,0);
        for (c.x = ba->ul.x +1; c.x < ba->size.x + ba->ul.x+1; c.x++) {
            for (c.y = ba->ul.y +1; c.y < ba->size.y + ba->ul.y+1; c.y++) {
                //if (dm_get_map_me(&c,map)->tile == ts_get_tile_specific(TILE_ID_CONCRETE_WALL) ) {
                    dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
                ////}
            }
        }
    }

    for (unsigned int a = 1; a < alist_sz; a++) {
        struct bsp_area *ba = &area_list[a];
        if (ba->split == true) continue;
        if (ba->route == true) continue;
        lg_debug("painting area[%d]: (sz %d,%d) (ul %d,%d) lvl: %d", a, ba->size.x, ba->size.y, ba->ul.x, ba->ul.y, ba->level);

        coord_t c = cd_create(0,0);
        for (c.x = ba->ul.x ; c.x < ba->size.x + ba->ul.x; c.x++) {
            for (c.y = ba->ul.y ; c.y < ba->size.y + ba->ul.y; c.y++) {
                dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
            }
        }

        c = ba->ul;
        for (c.x = ba->ul.x; c.x < ba->ul.x + ba->size.x +1; c.x++) {
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
        }
        c = ba->ul;
        for (c.y = ba->ul.y; c.y < ba->ul.y + ba->size.y +1; c.y++) {
            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
        }

        /*
        coord_t zero = cd_create(0,0);
        if (cd_equal(&ba->wall.a, &zero) == false) {
            lg_debug("removing wall between (%d,%d) and (%d,%d)", ba->wall.a.x, ba->wall.a.y, ba->wall.b.x, ba->wall.b.y);

            c = ba->wall.a;
            for (c.x = ba->wall.a.x +1; c.x < ba->wall.b.x ; c.x++) {
                lg_debug("placing dummy at (%d,%d)", c.x,c.y);
                dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_DUMMY);
            }
            c = ba->wall.a;
            for (c.y = ba->wall.a.y +1; c.y < ba->wall.b.y ; c.y++) {
                lg_debug("placing dummy at (%d,%d)", c.x,c.y);
                dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_DUMMY);
            }
        }
        */
    }

    for (unsigned int a = 1; a < alist_sz; a++) {
        struct bsp_area *ba = &area_list[a];
        coord_t zero = cd_create(0,0);
        if (cd_equal(&ba->entry, &zero) ) continue;
        dm_get_map_me(&ba->entry,map)->tile = ts_get_tile_specific(TILE_ID_WOODEN_CLOSED_DOOR);
    }

    for (int i = 0; i < nr_influence_points; i++) {
        coord_t ip = ip_list[i];
        dm_get_map_me(&ip,map)->tile = ts_get_tile_specific(TILE_ID_BRASSIER);
    }

    dm_get_map_me(&start,map)->tile = ts_get_tile_specific(TILE_ID_STAIRS_UP);
    dm_get_map_me(&end,map)->tile = ts_get_tile_specific(TILE_ID_STAIRS_DOWN);
    map->stair_up = start;
    map->stair_up = end;

    pf_exit(pf_ctx);
    free(coord_lst);
    return true;
}
