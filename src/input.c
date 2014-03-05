#include <ncurses.h>

#include "input.h"

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
        case KEY_HOME:  return INP_KEY_UP_LEFT; break;
        case KEY_UP:    return INP_KEY_UP; break;
        case KEY_NPAGE: return INP_KEY_UP_RIGHT; break;
        case KEY_RIGHT: return INP_KEY_RIGHT; break;
        case KEY_PPAGE: return INP_KEY_DOWN_RIGHT; break;
        case KEY_DOWN:  return INP_KEY_DOWN; break;
        case KEY_END:   return INP_KEY_DOWN_LEFT; break;
        case KEY_LEFT:  return INP_KEY_LEFT; break;

        case 'q':
        case 27:        return INP_KEY_ESCAPE; break;

        case 'i':       return INP_KEY_INVENTORY; break;
        case 'x':       return INP_KEY_EXAMINE; break;
        case 'f':       return INP_KEY_FIRE; break;
        case 'd':       return INP_KEY_DROP; break;
        case 'w':       return INP_KEY_WEAR; break;
        case 'u':       return INP_KEY_USE; break;
        case 'r':       return INP_KEY_RELOAD; break;
        case '[':       return INP_KEY_WEAPON_SETTING; break;
        case ']':       return INP_KEY_WEAPON_SELECT; break;

        case ' ':
        case '\n':
        case 'y':       return INP_KEY_YES; break;
        case 'n':       return INP_KEY_NO; break;
        case 'a':       return INP_KEY_ALL; break;

        case ',':
        case 'g':       return INP_KEY_PICKUP; break;

        case '>':       return INP_KEY_STAIRS_DOWN; break;
        case '<':       return INP_KEY_STAIRS_UP; break;
        default:
            break;
    }
    return INP_KEY_NONE;
}
