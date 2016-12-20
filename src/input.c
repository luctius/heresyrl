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

#include <ncurses.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>

#include "input.h"
#include "options.h"
#include "logging.h"
#include "ui/ui.h"

#define INP_KEYLOG_INCREASE 100

#define INPUT_PRE_CHECK (56325)
#define INPUT_POST_CHECK (41267)
static bool inp_verify(struct inp_input *i) {
    assert(i != NULL);
    assert(i->pre == INPUT_PRE_CHECK);
    assert(i->post == INPUT_POST_CHECK);
    if (i == NULL) return false;
    if (i->pre  != INPUT_PRE_CHECK)  return false;
    if (i->post != INPUT_POST_CHECK) return false;
    return true;
}

static bool inp_resize_log(struct inp_input *i) {
    if (inp_verify(i) == false) return false;

    int old = i->keylog_sz;
    i->keylog_sz += INP_KEYLOG_INCREASE;
    i->keylog = realloc(i->keylog, i->keylog_sz * sizeof(enum inp_keys) );
    lg_debug("input increase: %d => %d", old, i->keylog_sz);
    if (i->keylog == NULL) return false;
    return true;
}

void inp_add_to_log(struct inp_input *i, enum inp_keys key) {
    if (inp_verify(i) == false) return;

    if (i->keylog_widx >= i->keylog_sz -2) {
        assert(inp_resize_log(i) );
    }

    i->keylog[i->keylog_widx++] = key;
}

bool inp_log_has_keys(struct inp_input *i) {
    if (inp_verify(i) == false) return false;
    /*lg_debug("keylog has %d unread key strokes", i->keylog_widx - i->keylog_ridx);*/

    if ( (i->keylog_ridx < i->keylog_widx) &&
         (i->keylog_widx < i->keylog_sz) ) {
        if (options.play_recording == true && options.play_delay > 0) usleep(options.play_delay * 1000);
        return true;
    }

    if (options.play_recording == true) {
        options.play_recording = false;
        options.refresh = true;

        update_screen();
        refresh();
    }

    return false;
}

enum inp_keys inp_get_from_log(struct inp_input *i) {
    inp_verify(i);

    return i->keylog[i->keylog_ridx++];
}

char inp_key_translate_idx(int idx) {
    if (idx <= 25) return idx+0x61;
    else if (idx <= 35) return idx+0x30;
    return -1;
}

int inp_input_to_idx(enum inp_keys k) {
    int ret = -1;
    /* A-Z -> 0 - 25*/
    if (isupper(k) ) ret = k - 0x41;

    /* a-z -> 0 - 25*/
    else if (islower(k) ) ret = k - 0x61;

    /* 0-9 -> 26 - 35*/
    else if (isdigit(k) ) ret = (k - 0x30) +26;

    return ret;
}

bool inp_keylog_stop(struct inp_input *i) {
    if (i == NULL) return false;
    if (inp_verify(i) == false) return NULL;

    i->keylog_ridx = i->keylog_widx;
    return true;
}

enum inp_keys inp_get_input_idx(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_ESCAPE;

    if (inp_log_has_keys(i) == false) {
        int ch = getch();

        k = inp_input_to_idx(ch);

        inp_add_to_log(i, k);
    }

    assert(inp_log_has_keys(i) );
    k = inp_get_from_log(i);
    lg_debug("key %d", k);
    return k;
}

enum inp_keys inp_get_input_text(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_ESCAPE;

    if (inp_log_has_keys(i) == false) {
        while ( (isalnum(k = getch() ) == false) && (k != ' ') && (k != '\n') && (k != KEY_BACKSPACE) ) {
            /*lg_debug("key %d", k);*/
        }
        inp_add_to_log(i, k);
    }

    assert(inp_log_has_keys(i) );
    k = inp_get_from_log(i);
    lg_debug("key text %d", k);
    return k;
}

static enum inp_keys inp_translate_key(int ch) {
    enum inp_keys k = INP_KEY_NONE;
    switch (ch) {
        case 'y': case 55:  case KEY_HOME:  k = INP_KEY_UP_LEFT; break;
        case 'k': case 56:  case KEY_UP:    k = INP_KEY_UP; break;
        case 'u': case 57:  case KEY_NPAGE: k = INP_KEY_UP_RIGHT; break;
        case 'l': case 54:  case KEY_RIGHT: k = INP_KEY_RIGHT; break;
        case 'n': case 51:  case KEY_PPAGE: k = INP_KEY_DOWN_RIGHT; break;
        case 'j': case 50:  case KEY_DOWN:  k = INP_KEY_DOWN; break;
        case 'b': case 49:  case KEY_END:   k = INP_KEY_DOWN_LEFT; break;
        case 'h': case 52:  case KEY_LEFT:  k = INP_KEY_LEFT; break;
        case 124: k = INP_KEY_DIR_COMB; break; /* TODO find differnt key for this */
        case '.': case 53:                  k = INP_KEY_WAIT; break;

        case '/':       k = INP_KEY_RUN; break;

        case 'q':
        case 'Q':
        case 27:        k = INP_KEY_ESCAPE; break;

        case '@':       k = INP_KEY_CHARACTER; break;
        case 'L':       k = INP_KEY_LOG; break;
        case 'I':
        case 'i':       k = INP_KEY_INVENTORY; break;
        case 'X':
        case 'x':       k = INP_KEY_EXAMINE; break;
        case 'F':
        case 'f':       k = INP_KEY_FIRE; break;
        case 'D':
        case 'd':       k = INP_KEY_DROP; break;
        case 'T':       k = INP_KEY_THROW_ITEM; break;
        case 't':       k = INP_KEY_THROW; break;
        case 'W':
        case 'w':       k = INP_KEY_WEAR; break;
        case 'a':       k = INP_KEY_APPLY; break;
        case 'r':       k = INP_KEY_RELOAD; break;
        case 'R':       k = INP_KEY_UNLOAD; break;
        case '[':       k = INP_KEY_WEAPON_SETTING; break;
        case ']':       k = INP_KEY_WEAPON_SELECT; break;
        case '\\':      k = INP_KEY_AMMO_SELECT; break;
        case '+':
        case '=':       k = INP_KEY_PLUS; break;
        case '_':
        case '-':       k = INP_KEY_MINUS; break;

        case ' ':
        case '\n':
        case 'O':
        case 'o':       k = INP_KEY_YES; break;
        case 'C':
        case 'c':       k = INP_KEY_NO; break;
        case 'A':       k = INP_KEY_ALL; break;

        case ',':
        case 'g':       k = INP_KEY_PICKUP; break;

        case '?':       k = INP_KEY_HELP; break;

        case '>':       k = INP_KEY_STAIRS_DOWN; break;
        case '<':       k = INP_KEY_STAIRS_UP; break;
        case 24:        k = INP_KEY_QUIT; break;
        case 9:         k = INP_KEY_TAB; break;

        default:
            lg_debug("key pressed: %d.", ch);
            break;
    }
    return k;
}

enum inp_keys inp_get_input(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_NONE;

    if (inp_log_has_keys(i) == false) {
        if (options.test_auto) {
            return INP_KEY_QUIT;
        }

        k = inp_translate_key(getch() );

        if (k == INP_KEY_DIR_COMB) {
            k = INP_KEY_NONE;

            int k1 = inp_translate_key(getch());
            if (k1 == INP_KEY_LEFT || k1 == INP_KEY_RIGHT || k1 == INP_KEY_UP || k1 == INP_KEY_DOWN) {
                int k2 = inp_translate_key(getch());
                if (k2 == INP_KEY_LEFT || k2 == INP_KEY_RIGHT || k2 == INP_KEY_UP || k2 == INP_KEY_DOWN) {
                    if      (k1 == INP_KEY_UP    && k2 == INP_KEY_LEFT)  k = INP_KEY_UP_LEFT;
                    else if (k1 == INP_KEY_LEFT  && k2 == INP_KEY_UP)    k = INP_KEY_UP_LEFT;
                    else if (k1 == INP_KEY_UP    && k2 == INP_KEY_RIGHT) k = INP_KEY_UP_RIGHT;
                    else if (k1 == INP_KEY_RIGHT && k2 == INP_KEY_UP)    k = INP_KEY_UP_RIGHT;
                    else if (k1 == INP_KEY_DOWN  && k2 == INP_KEY_LEFT)  k = INP_KEY_DOWN_LEFT;
                    else if (k1 == INP_KEY_LEFT  && k2 == INP_KEY_DOWN)  k = INP_KEY_DOWN_LEFT;
                    else if (k1 == INP_KEY_DOWN  && k2 == INP_KEY_RIGHT) k = INP_KEY_DOWN_RIGHT;
                    else if (k1 == INP_KEY_RIGHT && k2 == INP_KEY_DOWN)  k = INP_KEY_DOWN_RIGHT;
                }
            }
        }

        if (k != INP_KEY_QUIT) inp_add_to_log(i, k);
    }

    if (k != INP_KEY_QUIT) {
        assert(inp_log_has_keys(i) );
        k = inp_get_from_log(i);
        lg_debug("key text %d", k);
        return k;
    }

    return k;
}

struct inp_input *inp_init(void) {
    struct inp_input *i = calloc(1, sizeof(struct inp_input) );
    if (i != NULL) {
        i->pre = INPUT_PRE_CHECK;

        i->keylog = NULL;
        i->keylog_sz = 0;
        i->keylog_widx = 0;
        i->keylog_ridx = 0;

        i->post = INPUT_POST_CHECK;
        assert(inp_resize_log(i) == true);
    }

    return i;
}

void inp_exit(struct inp_input *i) {
    if (inp_verify(i) == false) return;

    if (i != NULL) {
        if (i->keylog_sz > 0) {
            free(i->keylog);
            free(i);
        }
    }
}

