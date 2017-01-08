#include <string.h>

#include "wizard/wizard_commands.h"
#include "wizard/wizard_mode.h"
#include "heresyrl_def.h"
#include "logging.h"

#include "wizard/wz_cmds_inc.c"

static void cmd_exit(char *input) {
    wz_mode_exit();
}

static void cmd_help(char *input);
static const struct wz_cmd wz_cmd_list[] = {
    {
        .name = "help",
        .descr = "displays all commands",
        .func = &cmd_help,
    },
    {
        .name = "exit",
        .descr = "exits wizard mode",
        .func = &cmd_exit,
    },
    {
        .name = "q",
        .descr = "exits wizard mode",
        .func = &cmd_exit,
    },
    {
        .name = "list_items",
        .descr = "lists all items",
        .func = &cmd_list_items,
    },
    {
        .name = "list_monsters",
        .descr = "lists all monsters",
        .func = &cmd_list_monsters,
    },
    {
        .name = "list_status_effects",
        .descr = "lists all status effects",
        .func = &cmd_list_status_effects,
    },
    {
        .name = "inspect_item",
        .descr = "show all relevant info of item. <param: uid>",
        .func = &cmd_inspect_item,
    },
    {
        .name = "inspect_monster",
        .descr = "show all relevant info of item. <param: uid>",
        .func = &cmd_inspect_monster,
    },
    {
        .name = "inspect_se",
        .descr = "show all relevant info of item. <param: uid>",
        .func = &cmd_inspect_se,
    },
    {
        .name = "astar",
        .descr = "testing a* pathfinding",
        .func = &cmd_test_a_star,
    },
    {
        .name = "fov_test",
        .descr = "tests fov",
        .func = &cmd_fov_test,
    },
    {
        .name = "show_map",
        .descr = "show complete map",
        .func = &cmd_show_map,
    },
    {
        .name = "refresh",
        .descr = "refreshes the screen",
        .func = &cmd_refresh,
    },

};

static void cmd_help(char *input) {
    for (int i = 0; i < ARRAY_SZ(wz_cmd_list); i++) {
        const struct wz_cmd *cmd = &wz_cmd_list[i];

        if (input != NULL) {
            if (strncmp(cmd->name, input, strlen(cmd->name) ) == 0) {
                lg_wizard("%10.10s \t %s", cmd->name, cmd->descr);
                return;
            }
        }
        else lg_wizard("%s \t%-30s", cmd->name, cmd->descr);
    }
}

/* ----------------------------------------------- */
const struct wz_cmd *wz_command_get(int i) {
    if (i < 0) return NULL;
    if (i >= ARRAY_SZ(wz_cmd_list) ) return NULL;
    return &wz_cmd_list[i];
}

int wz_command_sz() {
    return ARRAY_SZ(wz_cmd_list);
}
