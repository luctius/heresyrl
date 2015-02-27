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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/queue.h>

#include "pathfinding.h"
#include "heresyrl_def.h"

struct pf_entry {
    coord_t pos;
    struct pf_map_entity *pfme;
    TAILQ_ENTRY(pf_entry) entries;
};

static TAILQ_HEAD(pf_list, pf_entry) pf_list_head;

static void pf_list_init() {
    TAILQ_INIT(&pf_list_head);
}

static void pf_list_exit() {
    struct pf_entry *e = NULL;
    while (pf_list_head.tqh_first != NULL) {
        e = pf_list_head.tqh_first;
        TAILQ_REMOVE(&pf_list_head, pf_list_head.tqh_first, entries);
        free(e);
    }
}

static void pf_list_add_tail(coord_t *c, struct pf_map_entity *pfe) {
    struct pf_entry *e = malloc(sizeof(struct pf_entry) );
    e->pos = *c;
    e->pfme = pfe;

    TAILQ_INSERT_TAIL(&pf_list_head, e, entries);
}

static void pf_list_add_sort(coord_t *c, struct pf_map_entity *pfe) {
    struct pf_entry *e = malloc(sizeof(struct pf_entry) );
    e->pos = *c;
    e->pfme = pfe;

    struct pf_entry *np = pf_list_head.tqh_first;
    if (np == NULL || pfe->score < np->pfme->score) {
        TAILQ_INSERT_HEAD(&pf_list_head, e, entries);
        return;
    }

    for (np = pf_list_head.tqh_first; np != NULL; np = np->entries.tqe_next) {
        if (pfe->score < np->pfme->score) {
            TAILQ_INSERT_AFTER(&pf_list_head, np, e, entries);
            return;
        }
    }

    TAILQ_INSERT_TAIL(&pf_list_head, e, entries);
}

static void pf_list_remove(struct pf_entry *e) {
    TAILQ_REMOVE(&pf_list_head, e, entries);
    free(e);
}

static struct pf_entry *pf_list_next(struct pf_entry *prev) {
    if (prev == NULL) {
        if (pf_list_head.tqh_first != NULL) return pf_list_head.tqh_first;
        return NULL;
    }

    return prev->entries.tqe_next;
}

struct pf_map {
    coord_t size;
    struct pf_map_entity *map;
};

struct pf_context {
    struct pf_map map;
    struct pf_settings set;
    int maximum_distance;
};

static inline struct pf_map_entity *pf_get_index(coord_t *p, struct pf_map *map) {
    return &map->map[(p->x * map->size.y) + p->y];
}

static const coord_t pf_coord_lo_table[] = {
    {-1,-1}, {-1,0}, {-1,1}, 
    { 0,-1},         { 0,1}, 
    { 1,-1}, { 1,0}, { 1,1},
};

static bool pf_flood_map(struct pf_context *ctx, coord_t *point) {
    if (ctx == NULL) return false;
    if (point == NULL) return false;
    if (cd_within_bound(point, &ctx->map.size) == false) return false;

    struct pf_map *map = &ctx->map;

    pf_list_init();
    pf_list_add_tail(point, pf_get_index(point, map));

    /* We increase the map we process slightly each cycle to ease the burden not using a list. */
    bool has_open = true;
    while (has_open) {
        has_open = false;

        struct pf_entry *e = NULL;
        while ( (e = pf_list_next(NULL) ) != NULL) {
            coord_t current = e->pos;

            struct pf_map_entity *me = pf_get_index(&current, map);
            me->state = PF_ENTITY_STATE_CLOSED;

            //lg_debug("flooding (%d,%d) -> [st: %d/ cst: %d/dst: %d/scr: %d]", current.x, current.y, me->state, me->cost, me->distance, me->score);

            for (int i = 0; i < (int) coord_nhlo_table_sz; i++) {
                coord_t pos = { .x = current.x + coord_nhlo_table[i].x, .y = current.y + coord_nhlo_table[i].y, };
                if (cd_within_bound(&pos, &map->size) == false) continue;

                struct pf_map_entity *me_new = pf_get_index(&pos, map);
                if (me_new->cost == PF_BLOCKED) continue;

                /* Translate the coordinates from our perspective to the users perspective.  */
                coord_t pos_cbk = cd_add(&pos, &ctx->set.map_start);

                /* Call the user provided callback function */
                unsigned int cost = ctx->set.pf_traversable_callback(ctx->set.map, &pos_cbk);

                /* If the return is PF_BLOCKED, we will not look at it again. */
                if (cost == PF_BLOCKED) {
                    me_new->cost = PF_BLOCKED;
                    me_new->state = PF_ENTITY_STATE_CLOSED;
                    me_new->distance = PF_BLOCKED;
                    continue;
                }

                if ( ( (me->cost +cost) < me_new->cost) || (me_new->state == PF_ENTITY_STATE_FREE) ) {
                    me_new->cost = me->cost + cost;
                    me_new->distance = me->distance +1;
                    me_new->state = PF_ENTITY_STATE_OPEN;
                    me_new->score = me_new->cost + (pyth(pos.x - point->x, pos.y - point->y) * 2);
                    has_open = true;
                    pf_list_add_tail(&pos, me_new);
                    //lg_debug("adding (%d,%d) -> [st: %d/ cst: %d/dst: %d/scr: %d]", pos.x, pos.y, me_new->state, me_new->cost, me_new->distance, me_new->score);
                }
            }

            pf_list_remove(e);
        }
    }

    pf_list_exit();
    return true;
}

static bool pf_astar_loop(struct pf_context *ctx, coord_t *start, coord_t *end) {
    if (ctx == NULL) return false;
    if (end == NULL) return false;

    struct pf_map *map = &ctx->map;

    pf_list_add_sort(start, pf_get_index(start, map));

    struct pf_entry *entry;
    while ( (entry = pf_list_next(NULL) ) != NULL) {
        /* get best node*/
        coord_t point = entry->pos;
        struct pf_map_entity *me = entry->pfme;

        if ( (point.x == -1) || (point.y == -1) ) {
            /* failed to find any open node... */
            return false;
        }
        else if ( (point.x == end->x) && (point.y == end->y) ) {
            /* found our goal, yeey! */
            return true;
        }

        lg_debug("entering (%d,%d) -> [st: %d/ cst: %d/dst: %d/scr: %d]", point.x, point.y, me->state, me->cost, me->distance, me->score);

        me->state = PF_ENTITY_STATE_CLOSED;

        /* calculate around current point */
        coord_t pos, pos_cbk;
        for (int i = 0; i < coord_nhlo_table_sz; i++) {
            pos.x = coord_nhlo_table[i].x + point.x;
            pos.y = coord_nhlo_table[i].y + point.y;

            if (cd_within_bound(&pos, &map->size) == false) continue;
            struct pf_map_entity *me_new = pf_get_index(&pos, map);
            if (me_new->cost == PF_BLOCKED) continue;

            pos_cbk.x = pos.x +ctx->set.map_start.x;
            pos_cbk.y = pos.y +ctx->set.map_start.y;
            unsigned int cost = ctx->set.pf_traversable_callback(ctx->set.map, &pos_cbk);
            if (cost == PF_BLOCKED) {
                me_new->cost = PF_BLOCKED;
                me_new->state = PF_ENTITY_STATE_CLOSED;
                continue;
            }

            /* If the new cost is better, OR it was in the free state, update it */
            if ( ( (me->cost +cost) < me_new->cost) || (me_new->state == PF_ENTITY_STATE_FREE) ) {
                me_new->cost = cost + me->cost;
                me_new->score = me_new->cost + (pyth(pos.x - end->x, pos.y - end->y) * 2);
                me_new->distance = me->distance +1;
                me_new->state = PF_ENTITY_STATE_OPEN;
                pf_list_add_sort(&pos, me_new);
            }

            //lg_debug("tested (%d,%d) -> [st: %d/ cst: %d/dst: %d/scr: %d]", pos.x, pos.y, me_new->state, me_new->cost, me_new->distance, me_new->score);
        }

        pf_list_remove(entry);
    }

    return false;
}

static bool pf_backtrace(struct pf_map *map, coord_t *end, coord_t coord_lst[], int coord_lst_len) {
    if (map == NULL) return false;
    if (end == NULL) return false;
    if ( (end->x < 0) || (end->x >= map->size.x) ) return false;
    if ( (end->y < 0) || (end->y >= map->size.y) ) return false;

    lg_debug("backtrace length %d", coord_lst_len);

    coord_t point = { .x = end->x, .y = end->y, };
    for (int i = coord_lst_len; i > 0; i--) {
        struct pf_map_entity *me = pf_get_index(&point, map);
        if (coord_lst != NULL) {
            coord_lst[i-1].x = point.x;
            coord_lst[i-1].y = point.y;
        }

        lg_debug("entering p(%d,%d, [%d] ) -> [st: %d/ cst: %d/dst: %d]", point.x, point.y, i, me->state, me->cost, me->distance);
        if (me->cost >= PF_BLOCKED) return false;
        if (me->distance == 0) return true;

        unsigned int best = PF_BLOCKED;
        coord_t pos, best_pos;
        for (int j = 0; j < coord_nhlo_table_sz; j++) {
            pos.x = coord_nhlo_table[j].x + point.x;
            pos.y = coord_nhlo_table[j].y + point.y;
            me = pf_get_index(&pos, map);
            if (me->state != PF_ENTITY_STATE_FREE) {
                bool found_best = false;

                if (best > me->cost) {
                    found_best = true;
                }
                else if (best == me->cost) {
                    coord_t dp = cd_delta_abs(&pos, end);
                    coord_t dbp = cd_delta_abs(&best_pos, end);
                    if ( (dp.x + dp.y) < (dbp.x + dbp.y) ) {
                        found_best = true;
                    }
                }

                if (found_best) {
                    best = me->cost;
                    best_pos.x = pos.x;
                    best_pos.y = pos.y;
                }
            }
            //lg_debug("testing p(%d,%d) -> [st: %d/ cst: %d/dst: %d]", pos.x, pos.y, me->state, me->cost, me->distance);
        }

        point.x = best_pos.x;
        point.y = best_pos.y;
    }

    return false;
}

struct pf_context *pf_init(struct pf_settings *pf_set) {
    if (pf_set == NULL) return NULL;
    if (pf_set->pf_traversable_callback == NULL) return NULL;

    struct pf_context *ctx = calloc(1, sizeof(struct pf_context) );
    if (ctx != NULL) {
        memcpy(&ctx->set, pf_set, sizeof(struct pf_settings) );
    }

    return ctx;
}
void pf_exit(struct pf_context *ctx) {
    if (ctx == NULL) return;
    if (ctx->map.map != NULL) free(ctx->map.map);
    free(ctx);
}

struct pf_settings *pf_get_settings(struct pf_context *ctx) {
    if (ctx == NULL) return NULL;
    return &ctx->set;
}

bool pf_dijkstra_map(struct pf_context *ctx, coord_t *start) {
    if (ctx == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;

    struct pf_map *map = &ctx->map;
    if (map->map != NULL) free(map->map);

    map->size.x = ctx->set.map_end.x - ctx->set.map_start.x;
    map->size.y = ctx->set.map_end.y - ctx->set.map_start.y;
    map->map = calloc(map->size.x * map->size.y, sizeof(struct pf_map_entity) );
    if (map->map == NULL) return false;

    pf_get_index(start, map)->cost = 1;
    pf_get_index(start, map)->distance = 0;
    pf_get_index(start, map)->state = PF_ENTITY_STATE_OPEN;

    lg_debug("start at (%d,%d)", start->x,  start->y);
    return pf_flood_map(ctx, start);
}

int pf_astar_map(struct pf_context *ctx, coord_t *start, coord_t *end) {
    if (ctx == NULL) return false;
    if (start == NULL) return false;
    if (end == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    struct pf_map *map = &ctx->map;

    if (map->map != NULL) free(map->map);

    map->size.x = ctx->set.map_end.x - ctx->set.map_start.x;
    map->size.y = ctx->set.map_end.y - ctx->set.map_start.y;
    map->map = calloc(map->size.x * map->size.y, sizeof(struct pf_map_entity) );
    if (map->map == NULL) return false;

    pf_get_index(start, map)->cost = 1;
    pf_get_index(start, map)->distance = 0;
    pf_get_index(start, map)->score = pyth(start->x - end->x, start->y - end->y);
    pf_get_index(start, map)->state = PF_ENTITY_STATE_OPEN;

    lg_debug("start at (%d,%d)", start->x,  start->y);
    lg_debug("end at (%d,%d)", end->x,  end->y);

    /* Do astar filling */
    pf_list_init();
    bool retval = pf_astar_loop(ctx, start, end);
    pf_list_exit();
    return retval;
}

int pf_calculate_path(struct pf_context *ctx, coord_t *start, coord_t *end, coord_t **coord_lst) {
    int length = -1;
    if (ctx == NULL) return false;
    if (start == NULL) return false;
    if (end == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    if ( (ctx->set.map_end.x == 0) && (ctx->set.map_end.y == 0) ) return false;
    if (pf_get_index(start, &ctx->map)->state == PF_ENTITY_STATE_FREE) return false;
    if (pf_get_index(end, &ctx->map)->state == PF_ENTITY_STATE_FREE) return false;
    if (pf_get_index(start, &ctx->map)->cost != 1) return false;
    if (pf_get_index(start, &ctx->map)->distance != 0) return false;

    lg_debug("backtrace: start at (%d,%d)", start->x,  start->y);
    lg_debug("backtrace: end at (%d,%d)", end->x,  end->y);

    length = pf_get_index(end, &ctx->map)->distance+1;

    coord_t *list = NULL;
    if (coord_lst != NULL) {
        *coord_lst = calloc(length +1 , sizeof(coord_t) );
        list = *coord_lst;
    }

    if (pf_backtrace(&ctx->map, end, list, length) == false) {
        lg_debug("backtrace failed");
        length = -1;
    }
    else lg_debug("backtrace succes");

    if (length == -1) free(list);
    return length;
}

bool pf_calculate_reachability(struct pf_context *ctx) {
    if (ctx == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    if ( (ctx->set.map_end.x == 0) && (ctx->set.map_end.y == 0) ) return false;

    /* Find a starting point. */
    coord_t target;
    for (int xi = 0; xi < ctx->map.size.x; xi++) {
        for (int yi = 0; yi < ctx->map.size.y; yi++) {
            target.x = xi;
            target.y = yi;
            if (pf_get_index(&target, &ctx->map)->state == PF_ENTITY_STATE_FREE){
                target.x += ctx->set.map_start.x;
                target.y += ctx->set.map_start.y;

                if (ctx->set.pf_traversable_callback(ctx->set.map, &target) < PF_BLOCKED) {
                    lg_debug("fail at (%d,%d)", target.x,  target.y);
                    return false;
                }
            }
        }
    }
    return true;
}

bool pf_get_non_flooded_tile(struct pf_context *ctx, coord_t *target, coord_t *nft) {
    coord_t best = { .x = 0, .y = 0, };
    int best_dist = INT_MAX;

    if (ctx == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    if ( (ctx->set.map_end.x == 0) && (ctx->set.map_end.y == 0) ) return false;

    coord_t test;
    /* Find a starting point. */
    for (int xi = 0; xi < ctx->map.size.x; xi++) {
        for (int yi = 0; yi < ctx->map.size.y; yi++) {
            test.x = xi;
            test.y = yi;
            if (pf_get_index(&test, &ctx->map)->state == PF_ENTITY_STATE_FREE){
                coord_t t_cbk;
                t_cbk.x = test.x + ctx->set.map_start.x;
                t_cbk.y = test.y + ctx->set.map_start.y;

                if (ctx->set.pf_traversable_callback(ctx->set.map, &t_cbk) < PF_BLOCKED) {
                    int test_dist = cd_pyth(&test, target);
                    if (test_dist < best_dist ) {
                        best.x = test.x;
                        best.y = test.y;
                        best_dist = test_dist;
                    }
                }
            }
        }
    }

    if (best_dist != INT_MAX) {
        *nft = best;
        return true;
    }

    return false;
}

bool pf_get_closest_flooded_tile(struct pf_context *ctx, coord_t *target, coord_t *out) {
    coord_t best = { .x = 0, .y = 0, };
    int best_dist = INT_MAX;

    if (ctx == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    if ( (ctx->set.map_end.x == 0) && (ctx->set.map_end.y == 0) ) return false;

    coord_t test;
    /* Find a starting point. */
    for (int xi = 0; xi < ctx->map.size.x; xi++) {
        for (int yi = 0; yi < ctx->map.size.y; yi++) {
            test.x = xi;
            test.y = yi;
            if (pf_get_index(&test, &ctx->map)->state != PF_ENTITY_STATE_FREE) {
                int test_dist = cd_pyth(&test, target);
                if (test_dist < best_dist ) {
                    if (pf_get_index(&test, &ctx->map)->cost < PF_BLOCKED) {
                        best.x = test.x;
                        best.y = test.y;
                        best_dist = test_dist;
                    }
                }
            }
        }
    }

    if (best_dist != INT_MAX) {
        *out = best;
        return true;
    }

    return false;
}

struct pf_map_entity *pf_get_me(struct pf_context *ctx, coord_t *point) {
    if (ctx == NULL) return NULL;
    if (ctx->set.pf_traversable_callback == NULL) return NULL;
    if ( (ctx->set.map_end.x == 0) && (ctx->set.map_end.y == 0) ) return NULL;

    coord_t pf_point = { .x = point->x - ctx->set.map_start.x, .y = point->y - ctx->set.map_start.y, };
    if (cd_within_bound(&pf_point, &ctx->map.size) == false) return NULL;
    
    return pf_get_index(&pf_point, &ctx->map);
}

