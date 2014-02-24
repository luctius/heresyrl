#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "simple_dungeon.h"

struct sd_map *sd_alloc_map(int x_sz, int y_sz) {
    if (x_sz < 2) return NULL;
    if (y_sz < 2) return NULL;

    int sz = sizeof(struct sd_map) + ( (x_sz) * y_sz * sizeof(*(((struct sd_map*)0)->map)) ) ;
    struct sd_map *map = malloc(sz);
    if (map == NULL) return NULL;

    map->x_sz = x_sz;
    map->y_sz = y_sz;
    return map;
}

int sd_free_map(struct sd_map *map) {
    if (map == NULL) return EXIT_SUCCESS;
    free(map);
    return EXIT_SUCCESS;
}

int sd_print_map(struct sd_map *map) {
    if (map == NULL) return EXIT_FAILURE;
    if (map->x_sz < 2) return EXIT_FAILURE;
    if (map->y_sz < 2) return EXIT_FAILURE;
    if (map->map == NULL) return EXIT_FAILURE;

    for (int y = 0; y < map->y_sz; y++) {
        for (int x = 0; x < map->x_sz; x++) {
            putchar(SD_GET_INDEX(x,y,map).type);
        }
        putchar('\n');
    }
    putchar('\n');

    return EXIT_SUCCESS;
}

int sd_generate_map(struct sd_map *map) {
    if (map == NULL) return EXIT_FAILURE;
    if (map->x_sz < 2) return EXIT_FAILURE;
    if (map->y_sz < 2) return EXIT_FAILURE;
    if (map->map == NULL) return EXIT_FAILURE;

    for (int x = 0; x < map->x_sz; x++) {
        for (int y = 0; y < map->y_sz; y++) {
            if (y == 0 || y == map->y_sz -1) SD_GET_INDEX(x,y,map).type = SD_WALL;
            else if (x == 0 || x == map->x_sz -1) SD_GET_INDEX(x,y,map).type = SD_WALL;
            else SD_GET_INDEX(x,y,map).type = SD_FLOOR;
            SD_GET_INDEX(x,y,map).has_player = false;
        }
    }

    return EXIT_SUCCESS;
}
