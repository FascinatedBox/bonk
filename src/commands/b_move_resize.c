#include <getopt.h>
#include <stdio.h>

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
    "Usage: %s [options] [<window-arg>=%0] x y w h\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "-h, --help               display this help and exit\n"
    "\n"
    "If you use literal 'x' or 'y' for the x coordinates, then the current\n"
    "coordinate will be used. This is useful for moving the window along\n"
    "only one axis.\n"
    ;

void do_move_resize(bonk_state_t *b,
                    int screen,
                    xcb_window_t wid,
                    xcb_ewmh_moveresize_window_opt_flags_t flags,
                    uint32_t x,
                    uint32_t y,
                    uint32_t w,
                    uint32_t h)
{
    xcb_ewmh_request_moveresize_window(
            b->ewmh, screen, wid, XCB_GRAVITY_STATIC,
            XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER, flags, x, y, w, h);
}

#define LETTER_ARG(a, b, c) \
    if (a##_arg[0] == b && a##_arg[1] == '\0') \
        opt_flags &= ~XCB_EWMH_MOVERESIZE_WINDOW_##c;

int b_move_resize(bonk_state_t *b)
{
    int screen = 0;

    xcb_ewmh_moveresize_window_opt_flags_t opt_flags =
        XCB_EWMH_MOVERESIZE_WINDOW_X |
        XCB_EWMH_MOVERESIZE_WINDOW_Y |
        XCB_EWMH_MOVERESIZE_WINDOW_WIDTH |
        XCB_EWMH_MOVERESIZE_WINDOW_HEIGHT;

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_and_require_n(b, 4);

    char *x_arg = bonk_arg_next_unchecked(b);
    char *y_arg = bonk_arg_next_unchecked(b);
    char *h_arg = bonk_arg_next_unchecked(b);
    char *w_arg = bonk_arg_next_unchecked(b);
    char *x_end, *y_end, *h_end, *w_end;
    int x = strtol(x_arg, &x_end, 10);
    int y = strtol(y_arg, &y_end, 10);
    int h = strtol(h_arg, &h_end, 10);
    int w = strtol(w_arg, &w_end, 10);

    if (*x_end || *y_end || *h_end || *w_end ||
        x < 0 || y < 0 || h < 0 || w < 0) {
        fprintf(stderr, "bonk move-resize error: Invalid coordinates given to move-resize.\n");
        return 0;
    }

    BONK_FOREACH_WINDOW_DO {
        LETTER_ARG(x, 'x', X)
        LETTER_ARG(y, 'y', Y)
        LETTER_ARG(h, 'h', WIDTH)
        LETTER_ARG(w, 'w', HEIGHT)

        do_move_resize(b, screen, iter_window,
                      opt_flags, x, y, h, w);
    }

    return 1;
}
