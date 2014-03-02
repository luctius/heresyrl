#include <stdlib.h>
#include <math.h>
#include "pathfinding.h"
#include "heresyrl_def.h"

struct pf_map {
    struct pf_coord size;
    unsigned int *map;
};
#define PF_GET_INDEX(p, map_ptr) map_ptr->map[((p->x) * map_ptr->size.y) + (p->y)]

static const struct pf_coord pf_coord_lo_table[3][3] = {
    { {-1,-1}, {-1,0}, {-1,1}, },
    { { 0,-1}, { 0,0}, { 0,1}, },
    { { 1,-1}, { 1,0}, { 1,1}, },
};

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static bool pf_flood_map_point(struct pf_settings *pf_set, struct pf_map *map, struct pf_coord *point, 
                                unsigned int distance, unsigned int max_distance) {
    if (pf_set == NULL) return false;
    if (map == NULL) return false;
    if (point == NULL) return false;
    if ( (point->x < 0) || (point->x >= map->size.x) ) return false;
    if ( (point->y < 0) || (point->y >= map->size.y) ) return false;
    if (PF_GET_INDEX(point, map) == PF_BLOCKED) return false;

    struct pf_coord pos = { .x = point->x + pf_set->map_start.x, .y = point->y + pf_set->map_start.y, };
    unsigned int cost = pf_set->pf_traversable_callback(pf_set->map, &pos);
    if (cost == PF_BLOCKED) {
        PF_GET_INDEX(point, map) = PF_BLOCKED;
        return false;
    }

    cost += distance;
    if ( (cost < PF_GET_INDEX(point, map) || (PF_GET_INDEX(point, map) == 0) ) ) {
        PF_GET_INDEX(point, map) = cost;
        //lg_printf("cost of (%d,%d): %d/%d", point->x,  point->y, PF_GET_INDEX( (point), (map) ), max_distance);

        if (cost < max_distance) {
            //for (int xi = 3; xi > 0; xi--) {
                //for (int yi = 3; yi > 0; yi--) {
            for (int xi = 0; xi < 3; xi++) {
                for (int yi = 0; yi < 3; yi++) {
                    pos.x = pf_coord_lo_table[xi][yi].x + point->x;
                    pos.y = pf_coord_lo_table[xi][yi].y + point->y;
                    if ( (cost < PF_GET_INDEX( (&pos), map) || (PF_GET_INDEX( (&pos), map) == 0) ) ) {
                        pf_flood_map_point(pf_set, map, &pos, cost, max_distance);
                    }
                }
            }
        }
    }
    return true;
}

static int pf_backtrace(struct pf_map *map, struct pf_coord *point, struct pf_coord coord_lst[], int coord_lst_idx) {
    if (map == NULL) return -1;
    if (point == NULL) return -1;
    if (coord_lst == NULL) return -1;

    unsigned int best_cost = PF_BLOCKED;
    for (int xi = 0; xi < 3; xi++) {
        for (int yi = 0; yi < 3; yi++) {
            struct pf_coord pos = { .x = pf_coord_lo_table[xi][yi].x + point->x, .y = pf_coord_lo_table[xi][yi].y + point->y, };
            if (PF_GET_INDEX( (&pos), map) < best_cost) {
                coord_lst[coord_lst_idx].x = pos.x;
                coord_lst[coord_lst_idx].y = pos.y;
                best_cost = PF_GET_INDEX( (&pos), map);
            }
        }
    }
    if (best_cost != PF_BLOCKED) {
        return pf_backtrace(map, &coord_lst[coord_lst_idx], coord_lst, coord_lst_idx +1);
    }

    return coord_lst_idx;
}

bool pf_calculate_reachability(struct pf_settings *pf_set) {
    if (pf_set == NULL) return false;
    if (pf_set->pf_traversable_callback == NULL) return false;
    struct pf_map map;
    map.size.x = pf_set->map_end.x - pf_set->map_start.x;
    map.size.y = pf_set->map_end.y - pf_set->map_start.y;
    map.map = calloc(map.size.x * map.size.y, sizeof(unsigned int) );
    if (map.map == NULL) return false;
    int max_cost = (pf_set->max_traversable_cost != 0) ? pf_set->max_traversable_cost : 1;
    int max_distance = max_cost * (max(map.size.x, map.size.y) );

    /* Find a starting point. */
    bool found_start = false;
    struct pf_coord cbp;
    for (int xi = 0; xi < map.size.x; xi++) {
        for (int yi = 0; yi < map.size.y; yi++) {
            cbp.x = xi + pf_set->map_start.x; 
            cbp.y = yi + pf_set->map_start.y;
            if (pf_set->pf_traversable_callback(pf_set->map, &cbp) < PF_BLOCKED) {
                cbp.x = xi; cbp.y = yi;
                found_start = true;
                xi = map.size.x;
                yi = map.size.y;
            }
        }
    }
    if (found_start) {
        pf_flood_map_point(pf_set, &map, &cbp, 0, max_distance);
        lg_printf("start at (%d,%d): %d", cbp.x,  cbp.y, PF_GET_INDEX( (&cbp), (&map) ) );

        for (int xi = 0; xi < map.size.x; xi++) {
            for (int yi = 0; yi < map.size.y; yi++) {
                cbp.x = xi;
                cbp.y = yi;
                if (PF_GET_INDEX( (&cbp), (&map) ) == 0){
                    cbp.x += pf_set->map_start.x;
                    cbp.y += pf_set->map_start.y;

                    /* point has not been touched, see if it is a wall. */
                    if (pf_set->pf_traversable_callback(pf_set->map, &cbp) < PF_BLOCKED) {
                        lg_printf("fail at (%d,%d): %d", cbp.x,  cbp.y, PF_GET_INDEX( (&cbp), (&map) ) );
                        free(map.map);
                        return false;
                    }
                }
            }
        }
    }
    free(map.map);
    return true;
}

int pf_calculate_path(struct pf_settings *pf_set, struct pf_coord *start, struct pf_coord *end, struct pf_coord **coord_lst) {
    int length = -1;
    if (pf_set == NULL) return -1;
    if (pf_set->pf_traversable_callback == NULL) return -1;
    struct pf_map map;
    map.size.x = pf_set->map_end.x - pf_set->map_start.x;
    map.size.y = pf_set->map_end.y - pf_set->map_start.y;
    map.map = calloc(map.size.x * map.size.y, sizeof(unsigned int) );
    if (map.map == NULL) return -1;
    int max_cost = (pf_set->max_traversable_cost != 0) ? pf_set->max_traversable_cost : 1;
    int max_distance = max_cost * (2 * max(map.size.x, map.size.y) );

    //check if start or end is PF_BLOCKED

    /* Find a starting point. */
    struct pf_coord pf_start, pf_end;
    pf_start.x = start->x - pf_set->map_start.x;
    pf_start.y = start->y - pf_set->map_start.y;
    pf_end.x = end->x - pf_set->map_end.x;
    pf_end.y = end->y - pf_set->map_end.y;

    pf_flood_map_point(pf_set, &map, &pf_start, 0, max_distance);
    length = PF_GET_INDEX( (&pf_end), (&map) );
    if (length < 1) return -1;

    if (coord_lst != NULL) {
        *coord_lst = calloc(length, sizeof(struct pf_coord));
        if (pf_backtrace(&map, &pf_start, *coord_lst, 0) == false) {
            length = -1;
        }
    }
    free(map.map);
    if (length < 1) free(*coord_lst); 
    return length;
}
