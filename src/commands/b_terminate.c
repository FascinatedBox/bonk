#include <getopt.h>

#include "bonk_internal.h"

typedef enum {
    opt_help,
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
    "\n"
    "Terminate program associated with a window. Similar to xkill.\n"
    "\n"
    "In most cases, the close command is a better option.\n"
    "It only targets one window, and it can be handled gracefully.\n"
    "\n"
    "Termination cannot be ignored.\n"
    "Terminating a window causes the window's creator (client) to close down.\n"
    "ALL windows associated with the client are destroyed!\n"
    "\n"
    "--wait                   flush output buffer before next command\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    ;

int b_terminate(bonk_state_t *b)
{
    int wait = 0;

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_only(b);

    BONK_FOREACH_WINDOW_DO(
        xcb_kill_client(b->conn, iter_window);
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
