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

#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <uncursed.h>

#include "input.h"
#include "options.h"
#include "logging.h"
#include "ui/ui.h"
#include "random.h"

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

static enum inp_keys inp_translate_key(wchar_t ch);

static bool inp_resize_log(struct inp_input *i) {
    if (inp_verify(i) == false) return false;

    int old = i->keylog_sz;
    i->keylog_sz += INP_KEYLOG_INCREASE;
    i->keylog = realloc(i->keylog, i->keylog_sz * sizeof(enum inp_keys) );
    lg_debug("input increase: %d => %d", old, i->keylog_sz);
    if (i->keylog == NULL) return false;
    return true;
}

static void inp_rnd_ai_fill_log(struct inp_input *i) {
    if (options.rnd_auto_play) {
        int max = 100 - inp_log_key_count(i);
        for (int c = max; c > 0; c--) {
            enum inp_keys k = random_int32(gbl_game->random) % INP_KEY_MAX;

            if (k == INP_KEY_QUIT) continue;
            inp_add_to_log(i, k);
        }
    }
}

void inp_add_to_log(struct inp_input *i, enum inp_keys key) {
    if (inp_verify(i) == false) return;

    if (i->keylog_widx >= i->keylog_sz -2) {
        assert(inp_resize_log(i) );
    }

    i->keylog[i->keylog_widx++] = key;
}

int inp_log_key_count(struct inp_input *i) {
    if (inp_verify(i) == false) return -1;
    /*lg_debug("keylog has %d unread key strokes", i->keylog_widx - i->keylog_ridx);*/
    return i->keylog_widx - i->keylog_ridx;
}

bool inp_log_has_keys(struct inp_input *i) {
    if (inp_verify(i) == false) return false;
    /*lg_debug("keylog has %d unread key strokes", i->keylog_widx - i->keylog_ridx);*/

    if ( (i->keylog_ridx < i->keylog_widx) &&
         (i->keylog_widx < i->keylog_sz) ) {
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

    if (options.play_recording == true && options.play_delay > 0) usleep(options.play_delay * 1000);
    return i->keylog[i->keylog_ridx++];
}

wchar_t inp_key_translate_idx(int idx) {
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

int inp_input_to_digit(enum inp_keys k) {
    int ret = -1;

    /* 0-9 -> 26 - 35*/
    if (isdigit(k) ) ret = (k - 0x30);

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

    inp_rnd_ai_fill_log(i);

    if (inp_log_has_keys(i) == false) {
        wint_t ch;
        int ret;
        ret = get_wch(&ch);

        if (ret == OK) {
            k = inp_input_to_idx(ch);
        }

        inp_add_to_log(i, k);
    }

    assert(inp_log_has_keys(i) );
    k = inp_get_from_log(i);
    /*lg_debug("key %d", k);*/
    return k;
}

wchar_t inp_get_input_text(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_ESCAPE;

    inp_rnd_ai_fill_log(i);

    if (inp_log_has_keys(i) == false) {
        int ret;

        bool get_input = true;
        do {
            ret = get_wch(&k);
            if (ret == KEY_CODE_YES) {
                if (k == KEY_BACKSPACE) get_input = false;
                k = inp_translate_key(k);
            }
            if (ret == OK) {
                if ( (k == KEY_ENTER) || (k == '\n') || (k == '\r') ) {
                    k = inp_translate_key(k);
                    get_input = false;
                }
                else if (isalnum(k) && k != ' ') get_input = false;
            }
            /*lg_debug("key %d", k);*/
        } while (get_input);
        inp_add_to_log(i, k);
    }

    assert(inp_log_has_keys(i) );
    k = inp_get_from_log(i);
    /*lg_debug("key text %d", k);*/
    return k;
}

/* TODO: make a different enum for normal text input with control characters, or do as libncurses does and treat special characters completely differently */
int inp_get_input_digit(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_ESCAPE;

    inp_rnd_ai_fill_log(i);

    if (inp_log_has_keys(i) == false) {
        int ret;

        bool get_input = true;
        do {
            ret = get_wch(&k);
            if (ret == KEY_CODE_YES) {
                if (k == KEY_BACKSPACE) get_input = false;
                k = inp_translate_key(k);
            }
            if (ret == OK) {
                if ( (k == KEY_ENTER) || (k == '\n') || (k == '\r') ) {
                    k = inp_translate_key(k);
                    get_input = false;
                }
                else if (isdigit(k) && k != ' ') get_input = false;
            }
            /*lg_debug("key %d", k);*/
        } while (get_input);
        inp_add_to_log(i, k);
    }

    assert(inp_log_has_keys(i) );
    k = inp_get_from_log(i);
    /*lg_debug("key text %d", k);*/
    return k;
}


static enum inp_keys inp_translate_key(wchar_t ch) {
    enum inp_keys k = INP_KEY_NONE;
    switch (ch) {
        case 'y': case KEY_A1: case KEY_HOME:   k = INP_KEY_UP_LEFT; break;
        case 'k': case KEY_A2: case KEY_UP:     k = INP_KEY_UP; break;
        case 'u': case KEY_A3: case KEY_PPAGE:  k = INP_KEY_UP_RIGHT; break;
        case 'h': case KEY_B1: case KEY_LEFT:   k = INP_KEY_LEFT; break;
        case 'l': case KEY_B3: case KEY_RIGHT:  k = INP_KEY_RIGHT; break;
        case 'n': case KEY_C3: case KEY_NPAGE:  k = INP_KEY_DOWN_RIGHT; break;
        case 'j': case KEY_C2: case KEY_DOWN:   k = INP_KEY_DOWN; break;
        case 'b': case KEY_C1: case KEY_END:    k = INP_KEY_DOWN_LEFT; break;
        case '.': case KEY_B2: case 53:                  k = INP_KEY_WAIT; break;

        case '/':       k = INP_KEY_RUN; break;

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

        case ':':       k = INP_KEY_WIZARD; break;

        case 124:           k = INP_KEY_DIR_COMB; break; /* TODO find differnt key for this */
        case 'q': case 'Q': case KEY_ESCAPE:    k = INP_KEY_ESCAPE; break;
        case '\f':          k = INP_KEY_REDRAW; break;
        case KEY_BACKSPACE: k = INP_KEY_BACKSPACE; break;

        case ' ':
        case '\n':
        case '\r':
        case 'O':
        case 'o':
        case KEY_ENTER:     k = INP_KEY_YES; break;

        default:
            /* lg_debug("key pressed: %d.", ch); */
            break;
    }
    return k;
}

enum inp_keys inp_get_input(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_NONE;

    inp_rnd_ai_fill_log(i);

    if (inp_log_has_keys(i) == false) {
        if (options.test_auto) {
            return INP_KEY_QUIT;
        }
        wint_t w;
        int ret = get_wch(&w);
        k = inp_translate_key(w);

        if (k == INP_KEY_DIR_COMB) {
            k = INP_KEY_NONE;

            ret = get_wch(&w);
            int k1 = inp_translate_key(w);
            if (k1 == INP_KEY_LEFT || k1 == INP_KEY_RIGHT || k1 == INP_KEY_UP || k1 == INP_KEY_DOWN) {
                ret = get_wch(&w);
                int k2 = inp_translate_key(w);
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
        /* lg_debug("key text %d", k); */
        return k;
    }

    return k;
}

void inp_init(void) {
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

    gbl_game->input = i;
}

void inp_exit(void) {
    struct inp_input *i = gbl_game->input;
    if (inp_verify(i) == false) return;

    if (i != NULL) {
        if (i->keylog_sz > 0) {
            free(i->keylog);
            free(i);
        }
    }
}

