#pragma once
#ifndef SIMPLE_DUNGEON_H_
#define SIMPLE_DUNGEON_H_

#define SD_WALL '#'
#define SD_FLOOR '.'
#define SD_DOOR '+'

struct sd_map_entity {
    char type;
    bool has_player;
};

struct sd_map {
    int x_sz;
    int y_sz;
    struct sd_map_entity map[];
};

#define SD_GET_INDEX(x,y, map_ptr) map_ptr->map[((x) * map->y_sz) + (y)]

struct sd_map *sd_alloc_map(int x_sz, int y_sz);
int sd_free_map(struct sd_map *map);

int sd_print_map(struct sd_map *map);
int sd_generate_map(struct sd_map *map);

#endif /*SIMPLE_DUNGEON_H_*/
