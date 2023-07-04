#ifndef BONK_INTERNAL_H
# define BONK_INTERNAL_H
# include <xcb/xcb.h>
# include <xcb/xcb_ewmh.h>

typedef struct {
    xcb_void_cookie_t *data;
    uint32_t pos;
    uint32_t size;
} bonk_cookie_jar;

typedef struct {
    xcb_window_t *data;
    uint32_t pos;
    uint32_t size;
} bonk_window_list;

typedef struct {
    xcb_connection_t *conn;
    xcb_ewmh_connection_t *ewmh;
    xcb_window_t *iter_windows;
    bonk_window_list *window_stack;
    const char *current_command;
    int iter_window_count;
    int argc;
    char **argv;
} bonk_state_t;

#define BONK_GETOPT_LOOP(c_, b_, shortargs, longopts) \
    int c_; \
 \
    while ((c_ = getopt_long_only(b_->argc, b_->argv, shortargs, longopts, NULL)) != -1)

#define BONK_GETOPT_COMMON \
    case opt_window: \
        bonk_use_window_arg(b, optarg); \
        continue; \
    BONK_GETOPT_HELP

#define BONK_GETOPT_HELP \
    case opt_help: \
        bonk_usage(b, usage); \
        break; \
    default: \
        return 0;

#define BONK_FOREACH_WINDOW_DO(action) \
    int w_index, w_count; \
    xcb_window_t *w_list = bonk_window_list_get(b, &w_count); \
    for (w_index = 0;w_index != w_count;w_index++) { \
        xcb_window_t iter_window = w_list[w_index]; \
        action \
    }

/* API functions for commands. */

bonk_window_list *bonk_new_window_list(int);
void bonk_free_window_list(bonk_window_list *);
void bonk_window_list_push(bonk_window_list *, xcb_window_t);

char *bonk_arg_next_unchecked(bonk_state_t *);
void bonk_arg_window_and_require_n(bonk_state_t *, int);
void bonk_arg_window_only(bonk_state_t *);

xcb_atom_t bonk_atom_find_existing(xcb_connection_t *, const char *);
xcb_atom_t bonk_atom_find_or_intern(xcb_connection_t *, const char *);

void bonk_connection_flush(bonk_state_t *);
void bonk_usage(bonk_state_t *, const char *);

xcb_window_t *bonk_window_list_get(bonk_state_t *, int *);

void bonk_use_window_arg(bonk_state_t *, const char *);
xcb_atom_t bonk_window_state_atom_from_string(bonk_state_t *, const char *);

#endif
