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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

#include "monster/monster.h"
#include "cmdline.h"

struct opt_options {
    bool debug;
    bool debug_show_map;
    bool debug_no_load;
    bool debug_no_save;
    bool print_map_only;

    bool rnd_auto_play;

    bool test_auto;
    bool test_mode;

    bool play_recording;
    int  play_delay;
    int  play_stop;
    bool refresh;

    char *log_file_name;
    char *save_file_name;
    char *load_file_name;

    char *char_name;

    enum homeworld_ids char_hw;
    enum background_ids char_bg;
    enum role_ids char_role;
};

extern struct opt_options options;

void opt_parse_options(struct gengetopt_args_info *args_info);
void opt_exit(void);

#endif /* OPTIONS_H */
