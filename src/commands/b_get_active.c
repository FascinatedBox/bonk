#include <getopt.h>
#include <stdio.h>

#include <xcb/xcb_ewmh.h>

#include "bonk_internal.h"

extern void bonk_arg_require_n(bonk_state_t *, int);

typedef enum {
    opt_help = 'h',
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options]\n"
    "\n"
    "Return the WM's active window (will probably focus and raise it)\n"
    "\n"
    "-h, --help               display this help and exit\n"
    ;

int b_get_active(bonk_state_t *b)
{
    BONK_GETOPT_LOOP(c, b, "+h", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_require_n(b, 0);

    xcb_get_property_cookie_t cookie = xcb_ewmh_get_active_window(b->ewmh, 0);
    xcb_window_t w;

    /* Unlike get-focus, this one seems to require a window manager to be
       present. This one can fail, but it's unlikely. */
    if (xcb_ewmh_get_active_window_reply(b->ewmh, cookie, &w, NULL) == 0)
        return 0;

    if (b->argc == 0)
        BONK_PRINT_WINDOW(w);
    else {
        b->window_stack->data[0] = w;
        b->window_stack->pos = 1;
    }

    return 1;
}
