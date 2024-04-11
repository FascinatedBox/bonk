#include <getopt.h>
#include <stdio.h>

#include "bonk_internal.h"
#include "xpick.h"

extern void bonk_arg_require_n(bonk_state_t *, int);

typedef enum {
    opt_help = 'h',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options]\n"
    "\n"
    "Select a window using a crosshair cursor\n"
    "\n"
    "Left click selects a window, any other mouse press cancels it.\n"
    "The selected window replaces the window stack.\n"
    "\n"
    "-h, --help               display this help and exit\n"
    ;

int b_pick(bonk_state_t *b)
{
    BONK_GETOPT_LOOP(c, b, "+h", longopts) {
        switch (c) {
            BONK_GETOPT_HELP
        }
    }

    bonk_arg_require_n(b, 0);

    xpick_state_t *s = xpick_state_new(b->conn);
    xcb_window_t window;

    if (xpick_cursor_grab(s, 0) == 0) {
        fprintf(stderr, "bonk pick error: Failed to grab the cursor.\n");
        return 0;
    }

    xpick_cursor_pick_window(s);
    xpick_cursor_ungrab(s);
    window = xpick_window_get(s);
    b->window_stack->data[0] = window;
    b->window_stack->pos = 1;
    xpick_state_free(s);

    if (b->argc == 0)
        BONK_PRINT_WINDOW(window);

    return 1;
}
