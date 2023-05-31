#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include <xcb/xcb_icccm.h>

#include "bonk_internal.h"

typedef enum {
    opt_wait,
    opt_help = 'h',
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

static void print_window_name(char *text, uint32_t len)
{
    /* Can't do `text[len] = '\0' because xcb doesn't like that (crashes). Use a
       new buffer each time because using a size-tracked buffer just isn't worth
       it here. */
    char *name = malloc((len + 1) * sizeof(*name));

    strncpy(name, text, len);
    name[len] = '\0';
    puts(name);
    free(name);
}

static void do_get_title(bonk_state_t *b, xcb_window_t wid)
{
    xcb_get_property_cookie_t c = xcb_ewmh_get_wm_name(b->ewmh, wid);

    {
        xcb_ewmh_get_utf8_strings_reply_t data;

        if (xcb_ewmh_get_wm_name_reply(b->ewmh, c, &data, NULL) != 0) {
            print_window_name(data.strings, data.strings_len);
            xcb_ewmh_get_utf8_strings_reply_wipe(&data);
            return;
        }
    }

    c = xcb_icccm_get_wm_name(b->conn, wid);

    {
        xcb_icccm_get_text_property_reply_t data;

        if (xcb_icccm_get_wm_name_reply(b->conn, c, &data, NULL) != 0) {
            print_window_name(data.name, data.name_len);
            xcb_icccm_get_text_property_reply_wipe(&data);
        }
    }
}

int b_get_title(bonk_state_t *b)
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
        do_get_title(b, iter_window);
    }

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
