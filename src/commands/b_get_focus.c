#include <getopt.h>
#include <stdio.h>

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
    "-h, --help               display this help and exit\n"
    ;

int b_get_focus(bonk_state_t *b)
{
    BONK_GETOPT_LOOP(c, b, "+h", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_require_n(b, 0);

    xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(b->conn);
    xcb_get_input_focus_reply_t *r = xcb_get_input_focus_reply(b->conn, cookie,
            NULL);

    if (r) {
        xcb_window_t w = r->focus;

        if (b->argc == 0)
            printf("0x%.8x\n", w);
        else {
            b->window_stack->data[0] = w;
            b->window_stack->pos = 1;
        }

        free(r);
    }

    return r != NULL;
}
