#include <getopt.h>

#include "bonk_internal.h"

typedef enum {
    opt_help = 'h',
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "window", required_argument, NULL, opt_window },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0] <property-name>\n"
    "\n"
    "Delete a property on a window\n"
    "\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "-h, --help               display this help and exit\n"
    ;

int b_prop_delete(bonk_state_t *b)
{
    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_and_require_n(b, 1);

    xcb_atom_t to_delete = bonk_atom_find_existing(b->conn,
            bonk_arg_next_unchecked(b));

    if (to_delete == 0)
        return 1;

    BONK_FOREACH_WINDOW_DO(
        xcb_delete_property(b->conn, iter_window, to_delete);
    )

    return 1;
}
