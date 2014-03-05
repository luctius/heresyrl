#ifndef UI_H
#define UI_H

#define MAP_MIN_COLS 20
#define MAP_MAX_COLS 100
#define MAP_COLS_FACTOR 0.90f

#define MAP_MIN_LINES 22
#define MAP_MAX_LINES 0
#define MAP_LINES_FACTOR 0.90f

#define MSG_MIN_COLS 40
#define MSG_MAX_COLS 40
#define MSG_COLS_FACTOR 0.95f

#define MSG_MIN_LINES 2
#define MSG_MAX_LINES 0
#define MSG_LINES_FACTOR 0.10f

#define CHAR_MIN_COLS 30
#define CHAR_MAX_COLS 30
#define CHAR_COLS_FACTOR 0.05f

#define CHAR_MIN_LINES 30
#define CHAR_MAX_LINES 30
#define CHAR_LINES_FACTOR 1.00f

#include "heresyrl_def.h"
#include "coord.h"

enum window_type {
    HRL_WINDOW_TYPE_MAP,
    HRL_WINDOW_TYPE_CHARACTER,
    HRL_WINDOW_TYPE_MESSAGE,
    HRL_WINDOW_TYPE_MAX,
};

struct hrl_window;

bool ui_create(int cols, int lines, struct hrl_window **map_win, struct hrl_window **char_win, struct hrl_window **msg_win);
void ui_destroy(struct hrl_window *map_win, struct hrl_window *char_win, struct hrl_window *msg_win);

void win_generate_colours(void);
struct hrl_window *win_create(int height, int width, int starty, int startx, enum window_type type);
void win_destroy(struct hrl_window *window);

void mapwin_display_map(struct hrl_window *window, struct dc_map *map, coord_t *player);
void mapwin_overlay_examine_cursor(struct hrl_window *mapwin, struct hrl_window *charwin, struct dc_map *map, coord_t *p_pos);
void mapwin_overlay_fire_cursor(struct hrl_window *window, struct pl_player *plr, struct dc_map *map, coord_t *p_pos);

void msgwin_log_callback(struct logging *log, struct log_entry *entry, void *priv);
void msgwin_log_refresh(struct hrl_window *window, struct logging *log);

void charwin_refresh(struct hrl_window *window, struct pl_player *plr);

void invwin_inventory(struct hrl_window *mapwin, struct hrl_window *charwin, struct dc_map *map, struct pl_player *plr);
#endif /* UI_H */
