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
    INP_KEY_CHARACTER,
    INP_KEY_PICKUP,
    INP_KEY_DROP,
    INP_KEY_USE,
    INP_KEY_THROW,
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
    INP_KEY_MAX,
};

struct inp_input {
    enum inp_keys *keylog;
    int keylog_sz;
    int keylog_widx;
    int keylog_ridx;
};

struct inp_input *inp_init(void);
void inp_exit(struct inp_input *i);
void inp_add_to_log(struct inp_input *i, enum inp_keys key);
enum inp_keys inp_get_from_log(struct inp_input *i);
bool inp_log_has_keys(struct inp_input *i);

enum inp_keys inp_get_input(struct inp_input *i);
enum inp_keys inp_get_input_idx(struct inp_input *i);
char inp_key_translate_idx(int idx);

#endif /* INPUT_H */
