#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>

#include <limits.h>

#include "config.h"
#include "options.h"

struct opt_options options = {
    .debug           = false,
    .debug_show_map  = false,
    .debug_no_load   = false,
    .debug_no_save   = false,
    .print_map_only  = false,

    .play_recording  = false,
    .play_delay      = 1,
    .play_stop       = 0,

    .refresh = true,

    .log_file_name   = NULL,
    .save_file_name  = NULL,

    .char_name      = NULL,
    .char_race      = MSR_RACE_MAX,
};

static const int path_max = PATH_MAX -1;

static char log_file[PATH_MAX -1];
static char save_file[PATH_MAX -1];

void opt_parse_options(struct gengetopt_args_info *args_info) {
    const char *homedir = getenv("HOME");
    if (homedir == NULL) {
        struct passwd *pw = getpwuid(getuid());
        homedir = pw->pw_dir;
    }

    options.debug           = args_info->debug_flag;
    options.debug_show_map  = args_info->map_flag;
    options.debug_no_load   = args_info->no_load_flag;
    options.debug_no_save   = args_info->no_save_flag;
    options.print_map_only  = args_info->print_map_only_flag;

    options.play_recording  = args_info->playback_flag;
    options.play_delay      = args_info->pb_delay_arg;
    options.play_stop       = args_info->pb_stop_arg;
    
    if (options.play_delay  == 0) options.refresh = false;

    if (args_info->log_file_given == false) {
        snprintf(log_file, path_max, "%s/.%s", homedir, PACKAGE_NAME);

        struct stat st;
        if ( (stat(log_file,&st) == 0) || (mkdir(log_file, 0777) >= 0) ) {
            snprintf(log_file, path_max, "%s/.%s/%s.log", homedir, PACKAGE_NAME, PACKAGE_NAME);
            options.log_file_name = log_file;
        }
    }

    if (args_info->save_file_given == false) {
        snprintf(save_file, path_max, "%s/.%s", homedir, PACKAGE_NAME);

        struct stat st;
        if ( (stat(save_file,&st) == 0) || (mkdir(save_file, 0777) >= 0) ) {
            snprintf(save_file, path_max, "%s/.%s/%s.save", homedir, PACKAGE_NAME, PACKAGE_NAME);
            options.save_file_name = save_file;
        }
    }

    if (args_info->name_given == true) {
        options.char_name = strdup(args_info->name_arg);
        options.debug_no_load = true;
    }
    if (args_info->race_given == true) {
        switch(args_info->race_arg) {
            case race__NULL:        options.char_race = MSR_RACE_MAX;
            case race_arg_dwarf:    options.char_race = MSR_RACE_DWARF;
            case race_arg_elf:      options.char_race = MSR_RACE_ELF;
            case race_arg_halfling: options.char_race = MSR_RACE_HALFLING;
            case race_arg_human:    options.char_race = MSR_RACE_HUMAN;
        };
        options.debug_no_load = true;
    }

    if (options.log_file_name == NULL) options.log_file_name = args_info->log_file_arg;
    if (options.save_file_name == NULL) options.save_file_name = args_info->save_file_arg;
    if (options.debug_no_load) options.play_recording = false;
}

