#pragma once
#ifndef SIMPLE_DUNGEON_H_
#define SIMPLE_DUNGEON_H_

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"
#include "tiles.h"

#define SD_GET_INDEX(x,y, map_ptr) map_ptr->map[((x) * map->y_sz) + (y)]
#define SD_GET_INDEX_TYPE(x,y, map_ptr) SD_GET_INDEX(x,y,map_ptr).tile.type

struct sd_map *sd_alloc_map(int x_sz, int y_sz);
int sd_free_map(struct sd_map *map);

int sd_print_map(struct sd_map *map);
int sd_generate_map(struct sd_map *map);
bool sd_tile_instance(struct sd_map *map, enum tile_types tt, int instance, int *xpos, int *ypos);

#endif /*SIMPLE_DUNGEON_H_*/
