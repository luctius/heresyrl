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

#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#include "coord.h"

#define PF_BLOCKED (INT_MAX / 2)

enum pf_entity_state {
    PF_ENTITY_STATE_FREE    = 0,
    PF_ENTITY_STATE_OPEN    = 1,
    PF_ENTITY_STATE_CLOSED  = 2,
};

struct pf_map_entity {
    int distance;
    unsigned int cost;
    unsigned int score;
    enum pf_entity_state state;
};

struct pf_settings {
    coord_t map_start;
    coord_t map_end;

    /* Pointer of local map structure */
    void *map;

    /* Should return the cost of the tile (normally 1 and always higher than zero)
       or PF_BLOCKED if it is a non-traversable tile.*/
    unsigned int (*pf_traversable_callback)(void *map, coord_t *coord);
};

struct pf_context;

/*
   Allocates memory for struct pf_context and copies
   the settings into them.
 */
struct pf_context *pf_init(struct pf_settings *pf_set);

/*
   Frees the pf_context struct and does deinitialisation.
 */
void pf_exit(struct pf_context *ctx);

/*
   Retreives the settings from within the pf_context struct.
 */
struct pf_settings *pf_get_settings(struct pf_context *ctx);

/*
Prepares a full dijkstra map of the area, calculating the distance to start from all the (reachable) points
The ctx can then be used by other functions.
*/
bool pf_dijkstra_map(struct pf_context *ctx, coord_t *start);

/*
Prereq: pf_dijkstra_map
Checks the map if every traversable square is reachable.
*/
bool pf_calculate_reachability(struct pf_context *ctx);

/*
Prereq: pf_dijkstra_map AND pf_calculate_reachability == false
Gives the coordinates of 'a' traversable tile which has not been
reached by dijkstra. Should only be called after pf_calculate_reachability.
Return true when it finds such a tile, else false.
*/
bool pf_get_non_flooded_tile(struct pf_context *ctx, coord_t *target, coord_t *nft);

/*
Prereq: pf_dijkstra_map
Gives the coordinates of a traversable tile which /has/ been reach
by dijkstra and it closest to the given tile.
return true on succes, false otherwise.
*/
bool pf_get_closest_flooded_tile(struct pf_context *ctx, coord_t *target, coord_t *out);

/*
Prepares a A* map of the area, calculating the distance to start from the squares it needs to reach end.
Faster then pf_dijkstra_map, but it will only calculate the path needed to reach end, most of the squares
will not be touched. Thus it cannot be used by others to reach the same goal.
*/
int pf_astar_map(struct pf_context *ctx, coord_t *start, coord_t *end);

/*
Prereq: pf_dijkstra_map OR pf_astar_map
Backtraces from end to start and finds the fastest route.
If the map is a dijkstra map, every traversable square can be used to travel to end.
If the map is an A* map, only squares along the route from start to end can be (reliably) used.
This means that a dijkstra map is reusable as long start remains the same.
    This means that calculations of other actors using the same map should be reversed.

coord_lst is the adres of an empty pf_coord pointer.
This will receive an adress via malloc and is the responsibility of the caller to be freed.
It is an array of pf_coord's of the calculated path.
If it is NULL it will not be used.

returns the length of the path
*/
int pf_calculate_path(struct pf_context *ctx, coord_t *start, coord_t *end, coord_t **coord_lst);

/*
Prereq: pf_dijkstra_map OR pf_astar_map
Retreive the information of a map grid.
*/
struct pf_map_entity *pf_get_me(struct pf_context *ctx, coord_t *point);

#endif /* PATHFINDING_H */
