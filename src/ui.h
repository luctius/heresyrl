#ifndef UI_H
#define UI_H

#define MAP_MIN_COLS 20
#define MAP_MAX_COLS 100
#define MAP_COLS_FACTOR 0.90f

#define MAP_MIN_LINES 22
#define MAP_MAX_LINES 0
#define MAP_LINES_FACTOR 0.90f

#define MSG_MIN_COLS 40
#define MSG_MAX_COLS 100
#define MSG_COLS_FACTOR 0.95f

#define MSG_MIN_LINES 2
#define MSG_MAX_LINES 0
#define MSG_LINES_FACTOR 0.10f

#define CHAR_MIN_COLS 31
#define CHAR_MAX_COLS 31
#define CHAR_COLS_FACTOR 0.05f

#define CHAR_MIN_LINES 30
#define CHAR_MAX_LINES 30
#define CHAR_LINES_FACTOR 1.00f

#include "heresyrl_def.h"
#include "coord.h"

bool ui_create(int cols, int lines);
void ui_destroy(void);

void mapwin_display_map(struct dc_map *map, coord_t *player);
void mapwin_overlay_examine_cursor(struct dc_map *map, coord_t *p_pos);
void mapwin_overlay_fire_cursor(struct gm_game *g, struct dc_map *map, coord_t *p_pos);

void msgwin_log_callback(struct logging *log, struct log_entry *entry, void *priv);
void msgwin_log_refresh(struct logging *log);

void charwin_refresh(struct pl_player *plr);

bool invwin_inventory(struct dc_map *map, struct pl_player *plr);

void ui_animate_projectile(struct dc_map *map, coord_t path[], int path_len, char projectile);
#endif /* UI_H */
