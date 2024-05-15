#include <getopt.h>
#include <stdio.h>

#include "bonk_internal.h"

typedef enum {
    opt_help,
    opt_raw,
    opt_wait,
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "raw", no_argument, NULL, opt_raw },
    { "wait", no_argument, NULL, opt_wait },
    { "window", required_argument, NULL, opt_window },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0] x y width height\n"
    "\n"
    "Move and resize a window (implemented as one action)\n"
    "\n"
    "--raw                    Use configure window to perform the move\n"
    "\n"
    "--wait                   flush output buffer before next command\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "\n"
    ""
    "If the corresponding letter (ex: 'x' for x) is used instead of a number, "
    "then\nthe current coordinate is used. This can be used to move a window "
    "along only one\naxis.\n"
    ;

static void do_raw_move_resize(bonk_state_t *b,
                               xcb_window_t wid,
                               xcb_ewmh_moveresize_window_opt_flags_t flags,
                               uint32_t x,
                               uint32_t y,
                               uint32_t w,
                               uint32_t h)
{
    uint16_t mask = 0;
    uint32_t values[] = {x, y, w, h};

    if (flags & XCB_EWMH_MOVERESIZE_WINDOW_X)
        mask |= XCB_CONFIG_WINDOW_X;

    if (flags & XCB_EWMH_MOVERESIZE_WINDOW_Y)
        mask |= XCB_CONFIG_WINDOW_Y;

    if (flags & XCB_EWMH_MOVERESIZE_WINDOW_WIDTH)
        mask |= XCB_CONFIG_WINDOW_WIDTH;

    if (flags & XCB_EWMH_MOVERESIZE_WINDOW_HEIGHT)
        mask |= XCB_CONFIG_WINDOW_HEIGHT;

    xcb_configure_window(b->conn, wid, mask, &values);
}

static void do_move_resize(bonk_state_t *b,
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
    int raw = 0, screen = 0, wait = 0;

    xcb_ewmh_moveresize_window_opt_flags_t opt_flags =
        XCB_EWMH_MOVERESIZE_WINDOW_X |
        XCB_EWMH_MOVERESIZE_WINDOW_Y |
        XCB_EWMH_MOVERESIZE_WINDOW_WIDTH |
        XCB_EWMH_MOVERESIZE_WINDOW_HEIGHT;

    uint32_t values[4] = {0, 0, 0, 0};

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            case opt_raw:
                raw = 1;
                break;
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_and_require_n(b, 4);

    char letter_for_arg[] = {'x', 'y', 'w', 'h'};
    char *letter_args[4];
    xcb_ewmh_moveresize_window_opt_flags_t flag_for_arg[4] = {
        XCB_EWMH_MOVERESIZE_WINDOW_X,
        XCB_EWMH_MOVERESIZE_WINDOW_Y,
        XCB_EWMH_MOVERESIZE_WINDOW_WIDTH,
        XCB_EWMH_MOVERESIZE_WINDOW_HEIGHT,
    };

    int valid = 1;

    for (int i = 0;i < 4;i++) {
        char *a = bonk_arg_take_next(b);
        char *a_end;
        int a_value = strtol(a, &a_end, 10);
        int ok = 1;

        if (a_value < -1 || a_value > 10000)
            ok = 0;
        else if (*a_end != '\0') {
            if (*a == letter_for_arg[i] && a[1] == '\0')
                opt_flags &= ~flag_for_arg[i];
            else
                ok = 0;
        }
        else
            values[i] = (uint32_t)a_value;

        if (ok == 0) {
            fprintf(stderr, "bonk move-resize error: Invalid %c coordinate (%s).\n",
                    letter_for_arg[i], a);
            valid = 0;
        }
    }

    if (valid == 0)
        return 0;

    uint32_t x = values[0],
             y = values[1],
             w = values[2],
             h = values[3];

    BONK_FOREACH_WINDOW_DO(
        if (raw == 0)
            do_move_resize(b, screen, iter_window, opt_flags, x, y, w, h);
        else
            do_raw_move_resize(b, iter_window, opt_flags, x, y, w, h);
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
