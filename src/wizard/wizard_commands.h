#ifndef WIZARD_COMMANDS_H
#define WIZARD_COMMANDS_H

struct wz_cmd {
    const char *name;
    const char *descr;
    void (*func)(char *input);
};

const struct wz_cmd *wz_command_get(unsigned int i);
int wz_command_sz(void);

#endif /* WIZARD_COMMANDS_H */
