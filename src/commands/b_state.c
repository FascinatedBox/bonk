#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "bonk_internal.h"

typedef enum {
    opt_wait,
    opt_add = 'a',
    opt_help = 'h',
    opt_remove = 'r',
    opt_toggle = 't',
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    {"add",    required_argument, NULL, opt_add},
    {"remove", required_argument, NULL, opt_remove},
    {"toggle", required_argument, NULL, opt_toggle},
    {"wait",   no_argument,       NULL, opt_wait},
    {"window", required_argument, NULL, opt_window},
    {"help",   no_argument,       NULL, opt_help},
    {NULL,     0,                 NULL, 0},
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0]\n"
    "--wait                   flush output buffer before continuing.\n"
    "-a, --add <property>     add a property\n"
    "-r, --remove <property>  remove a property\n"
    "-t, --toggle <property>  toggle a property\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "property can be one of \n"
    "ABOVE BELOW DEMANDS_ATTENTION FULLSCREEN HIDDEN MAXIMIZED_HORZ,\n"
    "MAXIMIZED_VERT MODAL SHADED SKIP_PAGER SKIP_TASKBAR STICKY\n"
    ;

void do_state(bonk_state_t *b,
              xcb_window_t iter_window,
              xcb_ewmh_wm_state_action_t action,
              xcb_atom_t target_property)
{
    xcb_ewmh_request_change_wm_state(b->ewmh,
                                     0,
                                     iter_window,
                                     action,
                                     target_property,
                                     /* xcb docs say this is a second property,
                                        but zero seems to always be right. */
                                     0,
                                     XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER);
}

#define MAX_AT_ONCE 8

int b_state(bonk_state_t *b)
{
    /* Valid values are from xcb_ewmh_wm_state_action_t. */
    xcb_atom_t atom_list[MAX_AT_ONCE];
    xcb_ewmh_wm_state_action_t action_list[MAX_AT_ONCE];
    int list_index = 0, wait = 0;

    BONK_GETOPT_LOOP(c, b, "+a:hr:t:w:", longopts) {
        int action = -1;

        switch (c) {
            case opt_add:
                action = XCB_EWMH_WM_STATE_ADD;
                break;
            case opt_remove:
                action = XCB_EWMH_WM_STATE_REMOVE;
                break;
            case opt_toggle:
                action = XCB_EWMH_WM_STATE_TOGGLE;
                break;
            case opt_wait:
                wait = 1;
                continue;
            BONK_GETOPT_COMMON
        }

        if (list_index == MAX_AT_ONCE) {
            fprintf(stderr, "bonk state error: Too many atoms to adjust at once (max = %d)\n",
                    MAX_AT_ONCE);
            return 0;
        }

        xcb_atom_t atom = bonk_window_state_atom_from_string(b, optarg);

        if (atom == XCB_ATOM_NONE) {
            fprintf(stderr, "bonk state error: '%s' is not a valid state atom. Stopping.\n",
                    optarg);
            return 0;
        }

        atom_list[list_index] = atom;
        action_list[list_index] = action;
        list_index++;
    }

    bonk_arg_window_only(b);

    /* User wants nothing done, and nothing was done. */
    if (list_index == 0)
        return 1;

    BONK_FOREACH_WINDOW_DO(
        int i;

        for (i = 0;i < list_index;i++)
            do_state(b, iter_window, action_list[i], atom_list[i]);
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
