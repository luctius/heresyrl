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

#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#include "config.h"

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif /*HAVE_PWD_H*/

#include <limits.h>

#include "options.h"

#ifdef AIMAKE_BUILDOS_MSWin32
#define hrl_mkdir(a,b) mkdir(a)
#else
#define hrl_mkdir(a,b) mkdir(a,b)
#endif

struct opt_options options = {
    .debug           = false,
    .debug_show_map  = false,
    .debug_show_monsters  = false,
    .debug_no_load   = false,
    .debug_no_save   = false,
    .print_map_only  = false,
    .test_auto       = false,
    .rnd_auto_play   = false,
    .wz_mode         = false,

    .play_recording  = false,
    .play_delay      = 100,
    .play_stop       = 0,

    .refresh = true,

    .log_file_name   = NULL,
    .save_file_name  = NULL,

    .char_name      = NULL,
    .char_hw        = CR_HWID_NONE,
    .char_bg        = CR_BCKGRNDID_NONE,
    .char_role      = CR_ROLEID_NONE,

    .path_max       = PATH_MAX-1,
};

static bool file_exist(const char *filename) {
    struct stat   buffer;
    return (stat (filename, &buffer) == 0);
}

void opt_exit(void) {
    free(options.log_file_name);
    free(options.save_file_name);
    free(options.load_file_name);
    free(options.char_name);
}

void opt_parse_options(struct gengetopt_args_info *args_info) {
    char *homedir = NULL;
#ifdef HAVE_PWD_H
    homedir = getenv("HOME");
    if (homedir == NULL) {
        struct passwd *pw = getpwuid(getuid());
        homedir = pw->pw_dir;
    }
#endif

    options.debug           = args_info->debug_flag;
    options.debug_show_map  = args_info->map_flag;
    options.debug_no_load   = args_info->no_load_flag;
    options.debug_no_save   = args_info->no_save_flag;
    options.print_map_only  = args_info->print_map_only_flag;
    options.test_auto       = args_info->test_auto_flag;
    options.test_mode       = args_info->test_mode_flag;
    options.rnd_auto_play   = args_info->rnd_auto_play_flag;
    options.wz_mode         = args_info->wizard_flag;

    options.play_recording  = args_info->playback_flag;
    options.play_delay      = args_info->pb_delay_arg;
    options.play_stop       = args_info->pb_stop_arg;

    if (args_info->log_file_given == false) {
        char *log_file = calloc(options.path_max, sizeof(char) );
        bool wlogf = false;

        if (homedir != NULL) {
            snprintf(log_file, options.path_max, "%s/.%s", homedir, PACKAGE_NAME);
            struct stat st;
            if ( (stat(log_file,&st) == 0) || (hrl_mkdir(log_file, 0700) >= 0) ) {
                snprintf(log_file, options.path_max, "%s/.%s/%s.log", homedir, PACKAGE_NAME, PACKAGE_NAME);
                wlogf = true;
            }
        }

        if (!wlogf) snprintf(log_file, options.path_max, "%s.log", PACKAGE_NAME);
        options.log_file_name = log_file;
    }

    if (args_info->save_file_given == false) {
        char *save_file = calloc(options.path_max, sizeof(char) );
        bool wsavef = false;

        if (homedir != NULL) {
            snprintf(save_file, options.path_max, "%s/.%s", homedir, PACKAGE_NAME);

            struct stat st;
            if ( (stat(save_file,&st) == 0) || (hrl_mkdir(save_file, 0700) >= 0) ) {
                snprintf(save_file, options.path_max, "%s/.%s/%s.save", homedir, PACKAGE_NAME, PACKAGE_NAME);
                wsavef = true;
            }
        }

        if (!wsavef) snprintf(save_file, options.path_max, "%s.save", PACKAGE_NAME);
        options.save_file_name = save_file;
    }
    if (args_info->load_file_given == true) {
        options.load_file_name = strdup(args_info->load_file_arg);
    }

    if (args_info->name_given == true) {
        int name_len = strlen(args_info->name_arg);
        wchar_t wname[name_len+2];
        mbstowcs(wname, args_info->name_arg, name_len);
        options.char_name = wcsdup(wname);
        options.debug_no_load = true;
    }
    if (args_info->homeworld_given == true) {
        switch(args_info->homeworld_arg) {
            case homeworld__NULL:       options.char_hw = CR_HWID_NONE;      break;
            case homeworld_arg_hive:    options.char_hw = CR_HWID_HIVE;     break;
            default: assert(false); break;
        }
        options.debug_no_load = true;
    }
    if (args_info->background_given == true) {
        switch(args_info->background_arg) {
            case background__NULL:       options.char_bg = CR_BCKGRNDID_NONE;           break;
            case background_arg_iguard:  options.char_bg = CR_BCKGRNDID_IMPERIAL_GUARD; break;
            case background_arg_outcast: options.char_bg = CR_BCKGRNDID_OUTCAST;        break;
            default: assert(false); break;
        }
        options.debug_no_load = true;
    }

    if (args_info->role_given == true) {
        switch(args_info->role_arg) {
            case role__NULL:       options.char_role = CR_ROLEID_NONE;    break;
            case role_arg_seeker:  options.char_role = CR_ROLEID_SEEKER;  break;
            case role_arg_warrior: options.char_role = CR_ROLEID_WARRIOR; break;
            default: assert(false); break;
        }
        options.debug_no_load = true;
    }

    if (options.log_file_name == NULL) options.log_file_name = strdup(args_info->log_file_arg);
    if (options.save_file_name == NULL) options.save_file_name = strdup(args_info->save_file_arg);
    if (options.load_file_name == NULL) options.load_file_name = strdup(options.save_file_name);
    if (options.debug_no_load) options.play_recording = false;

    if (options.rnd_auto_play) {
        options.play_recording = true;
    }
    if (options.test_auto) {
        options.play_recording = true;
        if (args_info->pb_delay_given == false) options.play_delay = 0;
    }

    if (options.play_delay == 0) options.refresh = false;


    if(file_exist(DATA_PATH "/fonts/font14.png") ) {
        options.data_path = DATA_PATH;
    } else {
        options.data_path = ".";
    }
}
