#define _XOPEN_SOURCE 700 // For strnlen()
#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#include "config.h"
#include "input.h"

#ifdef HAVE_LIBREADLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

#include <sys/param.h>

#include "wizard_mode.h"
#include "wizard_commands.h"
#include "logging.h"
#include "ui/ui.h"

static bool should_exit = false;

static bool wz_exists = false;

// Input character for readline
static unsigned char input;

// Used to signal "no more input" after feeding a character to readline
static bool input_avail = false;

#ifndef HAVE_LIBREADLINE
static void forward_to_readline(char c) {}
bool init_readline() { lg_error("Wizard mode not compiled in"); return false; }
void deinit_readline() {}
static void cmd_win_redisplay(bool for_resize) {}
void wz_init_rl() {}
#endif

char **wz_cmd_completion(const char *, int, int);
char *wz_cmd_generator(const char *, int);

// Calculates the cursor position for the readline window in a way that
// supports multibyte, multi-column and combining characters. readline itself
// calculates this as part of its default redisplay function and does not
// export the cursor position.
//
// Returns the total width (in columns) of the characters in the 'n'-byte
// prefix of the null-terminated multibyte string 's'. If 'n' is larger than
// 's', returns the total width of the string. Tries to emulate how readline
// prints some special characters.
//
// 'offset' is the current horizontal offset within the line. This is used to
// get tabstops right.
//
// Makes a guess for malformed strings.
static size_t strnwidth(const char *s, size_t n, size_t offset) {
    mbstate_t shift_state;
    wchar_t wc;
    size_t wc_len;
    size_t width = 0;

    // Start in the initial shift state
    memset(&shift_state, '\0', sizeof shift_state);

    size_t i = 0;
    for (i = 0; i < n; i += wc_len) {
        // Extract the next multibyte character
        wc_len = mbrtowc(&wc, s + i, MB_CUR_MAX, &shift_state);
        switch (wc_len) {
        case 0:
            // Reached the end of the string
            goto done;

        case (size_t)-1: case (size_t)-2:
            // Failed to extract character. Guess that the remaining characters
            // are one byte/column wide each.
            width += strnlen(s, n - i);

            goto done;
        }

        if (wc == '\t')
            width = ((width + offset + 8) & ~7) - offset;
        else
            // TODO: readline also outputs ~<letter> and the like for some
            // non-printable characters
            width += iswcntrl(wc) ? 2 : MAX(0, wcwidth(wc));
    }

done:
    return width;
}

// Like strnwidth, but calculates the width of the entire string
static size_t strwidth(const char *s, size_t offset) {
    return strnwidth(s, SIZE_MAX, offset);
}

// Not bothering with 'input_avail' and just returning 0 here seems to do the
// right thing too, but this might be safer across readline versions
static int readline_input_avail(void) {
    return input_avail;
}

static int readline_getc(FILE *dummy) {
    input_avail = false;

    return input;
}

#ifdef HAVE_LIBREADLINE
static void forward_to_readline(char c) {
    input = c;
    input_avail = true;
    rl_callback_read_char();
}

static void got_command(char *line) {
    bool success = false;

    if (line == NULL)
        // Ctrl-D pressed on empty line
        should_exit = true;
    else {
        int line_len = strlen(line);

        for (int i = 0; i < wz_command_sz(); i++) {
            const struct wz_cmd *cmd = wz_command_get(i);
            int cmd_len = strlen(cmd->name);
            if (strncmp(cmd->name, line, cmd_len) == 0) {
                add_history(line);

                for (int i = 0; i < line_len; i++ ) {
                    inp_add_to_log(gbl_game->input, line[i]);
                    inp_get_from_log(gbl_game->input);
                }
                inp_add_to_log(gbl_game->input, '\n');
                inp_get_from_log(gbl_game->input);


                char *param = NULL;
                if (cmd_len < line_len ) {
                    int i;
                    /* strip leading spaces */
                    for (i = cmd_len; i < line_len && isblank(line[i]); i++);
                    if (i < line_len) param = &line[i];
                }

                cmd->func(param);
                success = true;
                break;
            }
        }
    }
    
    if (success == false) lg_wizard("unkown command '%s'", line);
    free(line);
}

static void cmd_win_redisplay(bool for_resize) {
    size_t prompt_width = strwidth(rl_display_prompt, 0);
    size_t cursor_col = prompt_width +
                        strnwidth(rl_line_buffer, rl_point, prompt_width);

    werase(wz_win->win);
    // This might write a string wider than the terminal currently, so don't
    // check for errors
    mvwprintw(wz_win->win, 0, 0, "%s%s", rl_display_prompt, rl_line_buffer);
    if (cursor_col >= COLS)
        // Hide the cursor if it lies outside the window. Otherwise it'll
        // appear on the very right.
        curs_set(0);
    else {
        wmove(wz_win->win, 0, cursor_col);
        curs_set(2);
    }
    // We batch window updates when resizing
    wrefresh(wz_win->win);
}

static void readline_redisplay(void) {
    cmd_win_redisplay(false);
}

static bool init_readline(void) {
    // Disable completion. TODO: Is there a more robust way to do this?
    if (rl_bind_key('\t', rl_insert) != 0)
        lg_error("Invalid key passed to rl_bind_key()");

    // Let ncurses do all terminal and signal handling
    rl_catch_signals = 0;
    rl_catch_sigwinch = 0;
    rl_deprep_term_function = NULL;
    rl_prep_term_function = NULL;

    // Prevent readline from setting the LINES and COLUMNS environment
    // variables, which override dynamic size adjustments in ncurses. When
    // using the alternate readline interface (as we do here), LINES and
    // COLUMNS are not updated if the terminal is resized between two calls to
    // rl_callback_read_char() (which is almost always the case).
    rl_change_environment = 0;

    // Handle input by manually feeding characters to readline
    rl_getc_function = readline_getc;
    rl_input_available_hook = readline_input_avail;
    rl_redisplay_function = readline_redisplay;

    rl_callback_handler_install("> ", got_command);
    return true;
}

static void deinit_readline(void) {
    rl_callback_handler_remove();
}

static void init_ncurses(void) {
    cbreak();
    noecho();
    nonl();
    intrflush(NULL, FALSE);
    // Do not enable keypad() since we want to pass unadultered input to
    // readline

    // Explicitly specify a "very visible" cursor to make sure it's at least
    // consistent when we turn the cursor on and off (maybe it would make sense
    // to query it and use the value we get back too). "normal" vs. "very
    // visible" makes no difference in gnome-terminal or xterm. Let this fail
    // for terminals that do not support cursor visibility adjustments.
    curs_set(2);
}

char **wz_cmd_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, wz_cmd_generator);
}

void wz_init_rl() {
    rl_attempted_completion_function = wz_cmd_completion;
}
#endif

static void resize(void) {
    update_screen();

    cmd_win_redisplay(true);
}

char *wz_cmd_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    for (int i = list_index; i < wz_command_sz(); i++) {
        const struct wz_cmd *cmd = wz_command_get(i);
        if (strncmp(cmd->name, text, len) == 0) {
            lg_debug("completion('%s'): %s", text, cmd->name);
            list_index = i + 1;
            return strdup(cmd->name);
        }
    }

    return NULL;
}


void wz_init() {
    // Set locale attributes (including encoding) from the environment
    if (setlocale(LC_ALL, "") == NULL)
        lg_debug("Failed to set locale attributes from environment");

    wz_init_rl();
    wz_exists = init_readline();

    curs_set(0);
    werase(wz_win->win);
    wrefresh(wz_win->win);
}

void wz_exit() {
    deinit_readline();
    wz_exists = false;
}

void wz_mode() {
    should_exit = false;
    if (wz_exists == false) return;
    curs_set(2);

    werase(wz_win->win);
    wrefresh(wz_win->win);
    while (!should_exit) {
        // Using getch() here instead would refresh stdscr, overwriting the
        // initial contents of the other windows on startup
        int c;
        if (inp_log_has_keys(gbl_game->input) ) {
            c = inp_get_from_log(gbl_game->input);
        }
        else c = wgetch(wz_win->win);

        if (c == KEY_RESIZE)
            resize();
        else if (c == '\f') { // Ctrl-L -- redraw screen.
        }
        else if (c == 9) {
            rl_complete(0, '!');
        }
        else
            forward_to_readline(c);
    }
    curs_set(0);

    werase(wz_win->win);
    wrefresh(wz_win->win);
}

void wz_mode_exit() {
    should_exit = true;
}
