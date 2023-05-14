#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "bonk_internal.h"

typedef enum {
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
    {"window", required_argument, NULL, opt_window},
    {"help",   no_argument,       NULL, opt_help},
    {NULL,     0,                 NULL, 0},
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0]\n"
    "-a, --add <property>     add a property\n"
    "-t, --toggle <property>  toggle a property\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "property can be one of \n"
    "ABOVE BELOW DEMANDS_ATTENTION FULLSCREEN HIDDEN MAXIMIZED_HORZ,\n"
    "MAXIMIZED_VERT MODAL SHADED SKIP_PAGER SKIP_TASKBAR STICKY\n"
    ;

#define B_ABOVE              0
#define B_BELOW              1
#define B_DEMANDS_ATTENTION  2
#define B_FULLSCREEN         3
#define B_HIDDEN             4
#define B_MAXIMIZED_HORZ     5
#define B_MAXIMIZED_VERT     6
#define B_MODAL              7
#define B_SHADED             8
#define B_SKIP_PAGER         9
#define B_SKIP_TASKBAR      10
#define B_STICKY            11

static char *atom_table[] = {
    "ABOVE",
    "BELOW",
    "DEMANDS_ATTENTION",
    "FULLSCREEN",
    "HIDDEN",
    "MAXIMIZED_HORZ",
    "MAXIMIZED_VERT",
    "MODAL",
    "SHADED",
    "SKIP_PAGER",
    "SKIP_TASKBAR",
    "STICKY",
    NULL,
};

static xcb_atom_t name_to_wm_state_atom(bonk_state_t *b, const char *name)
{
    char *atom_name, upper_buffer[64];
    int i;

    bonk_strcpy_upper(upper_buffer, name);

    for (i = 0, atom_name = atom_table[i];
         atom_name != NULL;
         i++, atom_name = atom_table[i]) {

        if (atom_name[0] != upper_buffer[0] ||
            strcmp(atom_name, upper_buffer) != 0)
            continue;

        break;
    }

    /* Atoms start at 0, so this is an invalid atom. */
    if (atom_name == NULL)
        return 0;

    xcb_ewmh_connection_t *ewmh = b->ewmh;
    xcb_atom_t result;

    if (i == B_ABOVE)
        result = ewmh->_NET_WM_STATE_ABOVE;
    else if (i == B_BELOW)
        result = ewmh->_NET_WM_STATE_BELOW;
    else if (i == B_DEMANDS_ATTENTION)
        result = ewmh->_NET_WM_STATE_DEMANDS_ATTENTION;
    else if (i == B_FULLSCREEN)
        result = ewmh->_NET_WM_STATE_FULLSCREEN;
    else if (i == B_HIDDEN)
        result = ewmh->_NET_WM_STATE_HIDDEN;
    else if (i == B_MAXIMIZED_HORZ)
        result = ewmh->_NET_WM_STATE_MAXIMIZED_HORZ;
    else if (i == B_MAXIMIZED_VERT)
        result = ewmh->_NET_WM_STATE_MAXIMIZED_VERT;
    else if (i == B_MODAL)
        result = ewmh->_NET_WM_STATE_MODAL;
    else if (i == B_SHADED)
        result = ewmh->_NET_WM_STATE_SHADED;
    else if (i == B_SKIP_PAGER)
        result = ewmh->_NET_WM_STATE_SKIP_PAGER;
    else if (i == B_SKIP_TASKBAR)
        result = ewmh->_NET_WM_STATE_SKIP_TASKBAR;
    else
        result = ewmh->_NET_WM_STATE_STICKY;

    return result;
}

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
    int list_index = 0;

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
            BONK_GETOPT_COMMON
        }

        if (list_index == MAX_AT_ONCE) {
            fprintf(stderr, "bonk state error: Too many atoms to adjust at once (max = %d)\n",
                    MAX_AT_ONCE);
            return 0;
        }

        xcb_atom_t atom = name_to_wm_state_atom(b, optarg);

        if (atom == 0) {
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

    BONK_FOREACH_WINDOW_DO {
        int i;

        for (i = 0;i < list_index;i++)
            do_state(b, iter_window, action_list[i], atom_list[i]);
    }

    return 1;
}
