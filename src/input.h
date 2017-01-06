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

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>

#define INP_KEY_MAX_IDX (36)

enum inp_keys {
    INP_KEY_NONE,
    INP_KEY_UP_LEFT,
    INP_KEY_UP,
    INP_KEY_UP_RIGHT,
    INP_KEY_RIGHT,
    INP_KEY_DOWN_RIGHT,
    INP_KEY_DOWN,
    INP_KEY_DOWN_LEFT,
    INP_KEY_DIR_COMB,

    INP_KEY_RUN,

    INP_KEY_LEFT,
    INP_KEY_WAIT,
    INP_KEY_ESCAPE,
    INP_KEY_YES,
    INP_KEY_NO,
    INP_KEY_ALL,
    INP_KEY_EXAMINE,
    INP_KEY_FIRE,
    INP_KEY_RELOAD,
    INP_KEY_UNLOAD,
    INP_KEY_INVENTORY,
    INP_KEY_CHARACTER,
    INP_KEY_LOG,
    INP_KEY_PICKUP,
    INP_KEY_DROP,
    INP_KEY_APPLY,
    INP_KEY_THROW,
    INP_KEY_THROW_ITEM,
    INP_KEY_WEAR,
    INP_KEY_STAIRS_DOWN,
    INP_KEY_STAIRS_UP,
    INP_KEY_WEAPON_SETTING,
    INP_KEY_WEAPON_SELECT,
    INP_KEY_AMMO_SELECT,
    INP_KEY_PLUS,
    INP_KEY_MINUS,
    INP_KEY_TAB,
    INP_KEY_QUIT,
    INP_KEY_HELP,

    INP_KEY_MAX,
    INP_KEY_BACKSPACE,
};

struct inp_input {
    uint32_t pre;

    enum inp_keys *keylog;
    int keylog_sz;
    int keylog_widx;
    int keylog_ridx;

    uint32_t post;
};

struct inp_input *inp_init(void);
void inp_exit(struct inp_input *i);
void inp_add_to_log(struct inp_input *i, enum inp_keys key);
enum inp_keys inp_get_from_log(struct inp_input *i);
bool inp_log_has_keys(struct inp_input *i);
bool inp_keylog_stop(struct inp_input *i);

enum inp_keys inp_get_input(struct inp_input *i);
enum inp_keys inp_get_input_idx(struct inp_input *i);
char inp_key_translate_idx(int idx);
int inp_input_to_idx(enum inp_keys k);
enum inp_keys inp_get_input_text(struct inp_input *i);
enum inp_keys inp_get_input_digit(struct inp_input *i);
int inp_input_to_digit(enum inp_keys k);

#endif /* INPUT_H */
