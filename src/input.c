#include <ncurses.h>

#include "input.h"
#include "logging.h"

char inp_key_translate_idx(int idx) {
    if (idx <= 25) return idx+0x61;
    else if (idx <= 35) return idx+0x30;
    return -1;
}
enum inp_keys inp_get_input_idx(void) {
    char ch = getch();

    /* A-Z -> 0 - 25*/
    if (ch >= 0x41 && ch <= 0x5a) return ch - 0x41;

    /* a-z -> 0 - 25*/
    if (ch >= 0x61 && ch <= 0x7A) return ch - 0x61;

    /* 0-9 -> 26 - 35*/
    if (ch >= 0x30 && ch <= 0x39) return (ch - 0x30) +26;

    return INP_KEY_ESCAPE;
}

enum inp_keys inp_get_input(void) {
    int ch = getch();
    switch (ch) {
        case 'y': case 55:  case KEY_HOME:  return INP_KEY_UP_LEFT; break;
        case 'k': case 56:  case KEY_UP:    return INP_KEY_UP; break;
        case 'u': case 57:  case KEY_NPAGE: return INP_KEY_UP_RIGHT; break;
        case 'l': case 54:  case KEY_RIGHT: return INP_KEY_RIGHT; break;
        case 'n': case 51:  case KEY_PPAGE: return INP_KEY_DOWN_RIGHT; break;
        case 'j': case 50:  case KEY_DOWN:  return INP_KEY_DOWN; break; 
        case 'b': case 49:  case KEY_END:   return INP_KEY_DOWN_LEFT; break;
        case 'h': case 52:  case KEY_LEFT:  return INP_KEY_LEFT; break;
        case '.':                           return INP_KEY_WAIT; break;

        case 'q':
        case 27:        return INP_KEY_ESCAPE; break;

        case 'i':       return INP_KEY_INVENTORY; break;
        case 'x':       return INP_KEY_EXAMINE; break;
        case 'f':       return INP_KEY_FIRE; break;
        case 'd':       return INP_KEY_DROP; break;
        case 'w':       return INP_KEY_WEAR; break;
        case 'U':       return INP_KEY_USE; break;
        case 'r':       return INP_KEY_RELOAD; break;
        case 'R':       return INP_KEY_UNLOAD; break;
        case '[':       return INP_KEY_WEAPON_SETTING; break;
        case ']':       return INP_KEY_WEAPON_SELECT; break;

        case ' ':
        case '\n':
        case 'Y':       return INP_KEY_YES; break;
        case 'N':       return INP_KEY_NO; break;
        case 'A':
        case 'a':       return INP_KEY_ALL; break;

        case ',':
        case 'g':       return INP_KEY_PICKUP; break;

        case '>':       return INP_KEY_STAIRS_DOWN; break;
        case '<':       return INP_KEY_STAIRS_UP; break;
        case 24:        return INP_KEY_QUIT; break;
        case 9:         return INP_KEY_TAB; break;
                
        default:
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "main", "key pressed: %d.", ch);
            break;
    }
    return INP_KEY_NONE;
}
