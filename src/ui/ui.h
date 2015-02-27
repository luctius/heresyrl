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

#ifndef UI_H
#define UI_H

#include "heresyrl_def.h"
#include "coord.h"

bool ui_create(int cols, int lines);
void ui_destroy(void);

void update_screen(void);

void mapwin_display_map(struct dm_map *map, coord_t *player);
void mapwin_overlay_examine_cursor(struct dm_map *map, coord_t *p_pos);
bool mapwin_overlay_fire_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos);
bool mapwin_overlay_throw_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos);
bool mapwin_overlay_throw_item_cursor(struct gm_game *g, struct dm_map *map, coord_t *p_pos);

void msgwin_log_callback(struct logging *log, struct log_entry *entry, void *priv);
void msgwin_log_refresh(struct logging *log, struct log_entry *le);

void charwin_refresh(void);
void charwin_examine(const char *type, const char *name, const char *description);
bool invwin_inventory(struct dm_map *map, struct pl_player *plr);

void character_window(void);
void log_window(void);
void help_window(void);
bool char_creation_window(void);
void levelup_selection_window(void);

#endif /* UI_H */
