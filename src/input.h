#ifndef INPUT_H
#define INPUT_H

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
    INP_KEY_PICKUP,
    INP_KEY_DROP,
    INP_KEY_USE,
    INP_KEY_WEAR,
    INP_KEY_STAIRS_DOWN,
    INP_KEY_STAIRS_UP,
    INP_KEY_WEAPON_SETTING,
    INP_KEY_WEAPON_SELECT,
    INP_KEY_QUIT,
    INP_KEY_MAX,
};

enum inp_keys inp_get_input(void);
enum inp_keys inp_get_input_idx(void);
char inp_key_translate_idx(int idx);

#endif /* INPUT_H */
