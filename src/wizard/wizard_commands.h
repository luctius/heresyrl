#ifndef WIZARD_COMMANDS_H
#define WIZARD_COMMANDS_H

struct wz_cmd {
    char *name;
    char *descr;
    void (*func)(char *input);
};

const struct wz_cmd *wz_command_get(int i);
int wz_command_sz();

#endif /* WIZARD_COMMANDS_H */
