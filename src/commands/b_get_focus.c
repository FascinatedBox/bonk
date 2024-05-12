#include <getopt.h>
#include <stdio.h>

#include "bonk_internal.h"

typedef enum {
    opt_help,
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options]\n"
    "\n"
    "Return the window that currently has input focus\n"
    "\n"
    ;

static xcb_window_t get_pointer_root(bonk_state_t *b)
{
    xcb_screen_t **screens = b->ewmh->screens;
    int nb_screens = b->ewmh->nb_screens;
    xcb_window_t result = screens[0]->root;

    if (nb_screens == 1)
        return result;

    for (int i = 0;i < nb_screens;i++) {
        xcb_window_t root = screens[i]->root;

        /* Play Go Fish with X to find out which screen the pointer is on. */
        xcb_query_pointer_cookie_t c = xcb_query_pointer(b->conn, root);
        xcb_query_pointer_reply_t *r = xcb_query_pointer_reply(b->conn, c,
                NULL);

        if (r == NULL)
            continue;

        if (r->same_screen) {
            free(r);
            result = root;
            break;
        }
        else
            free(r);
    }

    return result;
}

int b_get_focus(bonk_state_t *b)
{
    BONK_GETOPT_LOOP(c, b, "+h", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON_NOWAIT
        }
    }

    bonk_arg_require_n(b, 0);

    xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(b->conn);
    xcb_get_input_focus_reply_t *r = xcb_get_input_focus_reply(b->conn, cookie,
            NULL);

    if (r) {
        xcb_window_t w = r->focus;

        /* This can return None (0x0), PointerRoot (0x1), or a valid wid. The
           middle one, PointerRoot, can be confusing to users since many X
           applications will complain about an invalid wid. To prevent that,
           figure out the right root and send that back. */
        if (w == XCB_INPUT_FOCUS_POINTER_ROOT)
            w = get_pointer_root(b);

        if (b->argc == 0)
            BONK_PRINT_WINDOW(w);
        else {
            b->window_stack->data[0] = w;
            b->window_stack->pos = 1;
        }

        free(r);
    }

    return r != NULL;
}
