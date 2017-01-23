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

#include "dungeon/dungeon_helpers.h"
#include "dungeon/cellular_automata.h"

bool dm_tunnel(struct dm_map *map, struct random *r, coord_t *start, coord_t *end, struct tl_tile *tl) {
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(start, &map->sett.size) == false) return false;
    if (cd_within_bound(end, &map->sett.size) == false) return false;
    if (tl == NULL) return false;

    bool first = true;
    bool tunnel_done = false;
    coord_t prev = { .x = start->x, .y = start->y, };

    while (!tunnel_done) {
        coord_t delta     = cd_delta(end, &prev);
        coord_t delta_abs = cd_delta_abs(end, &prev);

        int xd = 0;
        int yd = 0;
        int xmod = (delta.x >= 0) ? 1: -1;
        int ymod = (delta.y >= 0) ? 1: -1;

        bool last = false;
        if (cd_equal(&prev, end) || cd_neighbour(&prev, end) ) last = true;

        int roll = random_int32(r) % 100;
        if (first || last || roll < 80) {
            if (delta_abs.x >= delta_abs.y) {
                xd = 1 * xmod;
            }
            else if (delta_abs.x <= delta_abs.y) {
                yd = 1 * ymod;
            }
        }
        else {
            roll = random_int32(r) % 100;
            if (roll < 30) {
                xd = 1 * xmod;
            }
            else if (roll < 60) {
                yd = 1 * ymod;
            }
            else if (roll < 80) {
                xd = 1 * -xmod;
            }
            else if (roll < 100) {
                yd = 1 * -ymod;
            }
        }

        coord_t next = { .x = prev.x +xd, .y = prev.y +yd, };
        if (cd_within_bound(&next, &map->sett.size) ) {
            if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&next, map), TILE_ATTR_TRAVERSABLE) == false) {
                dm_get_map_me(&next, map)->tile = tl;
            }
            prev.x = next.x;
            prev.y = next.y;

            if (cd_equal(&prev, end) ) tunnel_done = true;
        }
        first = false;
    }
    return true;
}

bool dm_get_tunnel_path(struct dm_map *map, struct pf_context *pf_ctx, struct random *r, coord_t *tnl_start, coord_t *tnl_end) {
    if (dm_verify_map(map) == false) return false;

    /* get a coords from a place we did not reach with our flooding, nearest to the stairs*/
    coord_t nftl;
    if (pf_get_non_flooded_tile(pf_ctx, &map->stair_up, &nftl) == true) {

        /* get coords of a place we DID reach, so we can connect to that*/
        coord_t ftl;
        if (pf_get_closest_flooded_tile(pf_ctx, &nftl, &ftl) == true) {

            /* get the closest non-flooded tile to the flooded tile we just found. */
            coord_t nftl2;
            if (pf_get_non_flooded_tile(pf_ctx, &ftl, &nftl2) == true) {
                /* Tunnel our way to there.. */
                //struct tl_tile *tl = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
                struct tl_tile *tl = ts_get_tile_specific(TILE_ID_DUMMY);
                if (dm_tunnel(map, r, &ftl, &nftl2, tl) ) {
                    if (tnl_start != NULL) *tnl_start = ftl;
                    if (tnl_end   != NULL) *tnl_end   = nftl2;
                    lg_debug("tunneled from (%d,%d) to (%d,%d)", ftl.x, ftl.y, nftl2.x, nftl2.y);
                    return true;
                }
            }
        }
    }

    return false;
}

void dm_add_loops(struct dm_map *map, struct pf_context *pf_ctx, struct random *r) {
    if (dm_verify_map(map) == false) return;

    bool tunneled = true;
    for (int i = 0; i < 20 && tunneled == true; i++) {
        tunneled = false;

        if (aiu_generate_dijkstra(&pf_ctx, map, &map->stair_down, 0) ) {

            for (int x = 1; x < map->sett.size.x && tunneled == false; x+=5) {
                for (int y = 1; y < map->sett.size.y && tunneled == false; y+=5) {
                    coord_t point;

                    coord_t best;
                    coord_t worst;
                    int best_distance = INT_MAX;
                    int worst_distance = 0;
                    for (point.x = x - 5; point.x < x + 5; point.x++) {
                        for (point.y = y - 5; point.y < y + 5; point.y++) {
                            struct pf_map_entity *me = pf_get_me(pf_ctx, &point);
                            if ( (me != NULL) && (me->cost < PF_BLOCKED)  && me->state != PF_ENTITY_STATE_FREE) {
                                if (me->distance < best_distance)  {
                                    best_distance = me->distance;
                                    best = point;
                                }
                                if (me->distance > worst_distance) {
                                    worst_distance = me->distance;
                                    worst = point;
                                }
                            }
                        }
                    }

                    if ( (worst_distance - best_distance) > 20) {
                        struct tl_tile *tl = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
                        dm_tunnel(map, r, &best, &worst, tl);
                        tunneled = true;
                    }
                }
            }
        }
    }
}

void dm_add_lights(struct dm_map *map, struct random *r) {
    for (int x = 1; x < map->sett.size.x; x++) {
        for (int y = 1; y < map->sett.size.y; y++) {
            coord_t point = { .x = x, .y = y, };

            int num = 0;
            coord_t test = { .x = point.x, .y = point.y, };
            for (int i = 0; i < coord_nhlo_table_sz; i++) {
                if (cd_within_bound(&test, &map->sett.size) == false) continue;

                if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&test, map), TILE_ATTR_TRANSPARENT) == false) {
                    num++;
                }
            }


            struct dm_map_entity *me = dm_get_map_me(&point, map);
            if (me->tile->type == TILE_TYPE_FLOOR ) {
                if(random_int32(r)%100 < 1) {
                    me->tile = ts_get_tile_specific(TILE_ID_BRASSIER);
                    lg_debug("light at (%d,%d)", x,y);
                }
            }
        }
    }

}

bool dm_generate_feature(struct dm_map *map, struct random *r, coord_t *point, int min_radius, int max_radius, enum dm_feature_type ft) {
    coord_t ft_sz = { .x = max_radius, .y = max_radius, };
    coord_t ul = { .x = point->x - max_radius, .y = point->y - max_radius, };
    //coord_t dr = { .x = point->x + max_radius, .y = point->y + max_radius, };

    struct ca_map *camap = ca_init(&ft_sz);

    /* Insert Obstacles and Alive Members */
    coord_t c;
    for (c.x = 0; c.x < max_radius; c.x++) {
        for (c.y = 0; c.y < max_radius; c.y++) {
            coord_t p = cd_add(&c, &ul);

            if (cd_within_bound(&p, &map->sett.size) == false) {
                ca_set_coord(camap, &c, CA_OBSTACLE);
            }
            else if (dm_get_map_tile(&p, map)->type != TILE_TYPE_FLOOR) {
                ca_set_coord(camap, &c, CA_OBSTACLE);
            }
            else if (random_d100(r) < 70) {
                ca_set_coord(camap, &c, CA_ALIVE);
            }
        }
    }

    /* Generate the Feature */
    for (int i = 0; i < 3; i++) {
        ca_generation(camap, 4, 4, 1);
    }

    /* Check Size */
    int max_sum = 0;
    for (c.x = 0; c.x < max_radius; c.x++) {
        for (c.y = 0; c.y < max_radius; c.y++) {
            int tmp = 0;
            tmp = ca_get_coord_sum(camap, &c, 3);
            if (tmp > max_sum) max_sum = tmp;
        }
    }
    if (max_sum < min_radius) {
        ca_free(camap);
        return false;
    }

    /* Insert into Map */
    for (c.x = 0; c.x < max_radius; c.x++) {
        for (c.y = 0; c.y < max_radius; c.y++) {
            if (ca_get_coord(camap, &c) != CA_ALIVE) continue;

            coord_t p = cd_add(&c, &ul);

            if (cd_within_bound(&p, &map->sett.size) == false) continue;
            struct dm_map_entity *me = dm_get_map_me(&p, map);

            int sum = ca_get_coord_sum(camap, &c, 2);
            if (ft == DM_FT_POOL) {
                if (ca_get_coord_sum(camap, &c, 1) < 6) me->tile = ts_get_tile_specific(TILE_ID_MUD);
                else if (sum <= 19) me->tile = ts_get_tile_specific(TILE_ID_UNDEEP_WATER);
                else /*if (sum > 20)*/ me->tile = ts_get_tile_specific(TILE_ID_DEEP_WATER);
            }
        }
    }


    ca_free(camap);
    return true;
}

