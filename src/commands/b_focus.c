#include <getopt.h>
#include <stdio.h>

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
    "Give input focus to a window (will not raise it)\n"
    "\n"
    "--wait                   flush output buffer before next command\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    ;

int b_focus(bonk_state_t *b)
{
    int wait = 0;

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_only(b);

    BONK_FOREACH_WINDOW_DO(
        xcb_set_input_focus(b->conn,
                            XCB_INPUT_FOCUS_PARENT,
                            iter_window,
                            XCB_CURRENT_TIME);
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
