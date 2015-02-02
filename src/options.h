#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

#include "cmdline.h"

struct opt_options {
    bool debug;
    bool debug_show_map;
    bool debug_no_load;
    bool debug_no_save;
    bool print_map_only;

    bool play_recording;
    int  play_delay;
    int  play_stop;
    bool refresh;

    char *log_file_name;
    char *save_file_name;
};

extern struct opt_options options;

void opt_parse_options(struct gengetopt_args_info *args_info);

#endif /* OPTIONS_H */
