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
        if (options.play_recording == true) usleep(options.play_delay * 100000);
        return true;
    }

    if (options.play_recording == true) {
        options.play_recording = false;
        options.refresh = true;

        update_screen();
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
    return inp_get_from_log(i);
}

enum inp_keys inp_get_input_text(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_ESCAPE;

    if (inp_log_has_keys(i) == false) {
        while ( (isalpha(k = getch() ) == false) && (k != '\n') && (k != KEY_BACKSPACE) ) {
            /*lg_debug("key %d", k);*/
        }
        inp_add_to_log(i, k);
    }

    assert(inp_log_has_keys(i) );
    return inp_get_from_log(i);
}

enum inp_keys inp_get_input(struct inp_input *i) {
    if (i == NULL) return -1;
    if (inp_verify(i) == false) return -1;
    enum inp_keys k = INP_KEY_NONE;

    if (inp_log_has_keys(i) == false) {
        int ch = getch();
        switch (ch) {
            case 'y': case 55:  case KEY_HOME:  k = INP_KEY_UP_LEFT; break;
            case 'k': case 56:  case KEY_UP:    k = INP_KEY_UP; break;
            case 'u': case 57:  case KEY_NPAGE: k = INP_KEY_UP_RIGHT; break;
            case 'l': case 54:  case KEY_RIGHT: k = INP_KEY_RIGHT; break;
            case 'n': case 51:  case KEY_PPAGE: k = INP_KEY_DOWN_RIGHT; break;
            case 'j': case 50:  case KEY_DOWN:  k = INP_KEY_DOWN; break; 
            case 'b': case 49:  case KEY_END:   k = INP_KEY_DOWN_LEFT; break;
            case 'h': case 52:  case KEY_LEFT:  k = INP_KEY_LEFT; break;
            case '.': case 53:                  k = INP_KEY_WAIT; break;

            case 'q':
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
            case 'T':
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

            case '>':       k = INP_KEY_STAIRS_DOWN; break;
            case '<':       k = INP_KEY_STAIRS_UP; break;
            case 24:        k = INP_KEY_QUIT; break;
            case 9:         k = INP_KEY_TAB; break;
                    
            default:
                lg_debug("key pressed: %d.", ch);
                break;
        }
        if (k != INP_KEY_QUIT) inp_add_to_log(i, k);
    }

    if (k != INP_KEY_QUIT) {
        assert(inp_log_has_keys(i) );
        return inp_get_from_log(i);
    }

    return k;
}

struct inp_input *inp_init(void) {
    struct inp_input *i = malloc( sizeof(struct inp_input) );
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

