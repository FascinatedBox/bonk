#include <getopt.h>
#include <stdio.h>

#include "bonk_internal.h"

typedef enum {
    opt_help = 'h',
    opt_wait,
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "wait", no_argument, NULL, opt_wait },
    { "window", required_argument, NULL, opt_window },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0]\n"
    "--wait                   flush output buffer before continuing.\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "-h, --help               display this help and exit\n"
    ;

int b_map(bonk_state_t *b)
{
    int wait = 0;

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            case opt_wait:
                wait = 1;
                break;
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_only(b);

    BONK_FOREACH_WINDOW_DO {
        xcb_map_window(b->conn, iter_window);
    }

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
