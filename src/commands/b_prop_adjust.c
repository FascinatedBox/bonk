#include <getopt.h>
#include <string.h>

#include <xcb/xcb_icccm.h>

#include "bonk_internal.h"

typedef enum {
    opt_class,
    opt_instance,
    opt_title,
    opt_wait,
    opt_help = 'h',
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "class", required_argument, NULL, opt_class },
    { "instance", required_argument, NULL, opt_instance },
    { "title", required_argument, NULL, opt_title },
    { "wait", no_argument, NULL, opt_wait },
    { "window", required_argument, NULL, opt_window },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0]\n"
    "\n"
    "Adjust simple window properties\n"
    "\n"
    "--class <value>          Update the class in WM_CLASS.\n"
    "--instance <value>       Update the instance in WM_CLASS.\n"
    "--title <value>          Update the window's title (_NET_WM_NAME).\n"
    "\n"
    "--wait                   flush output buffer before next command\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "-h, --help               display this help and exit\n"
    ;

#define ADJUST_CLASS    0x1
#define ADJUST_INSTANCE 0x2
#define ADJUST_TITLE    0x4

char *get_wm_class_and_instance(bonk_state_t *b,
                                xcb_window_t window,
                                const char *class_name,
                                const char *instance_name,
                                int *len)
{
    int have_reply = 0;
    xcb_icccm_get_wm_class_reply_t r;

    /* The class and instance of a window are in the same field (WM_CLASS). If a
       user asks to change only one of them, then use the existing value of the
       other one. */
    if (class_name == NULL || instance_name == NULL) {
        xcb_get_property_cookie_t c = xcb_icccm_get_wm_class(b->conn, window);

        if (xcb_icccm_get_wm_class_reply(b->conn, c, &r, NULL) != 0) {
            have_reply = 1;
            if (class_name == NULL)
                class_name = r.class_name;
            else
                instance_name = r.instance_name;
        }
        else {
            /* For some reason, this window doesn't have WM_CLASS set, so use a
               blank string for the missing property. */
            if (class_name == NULL)
                class_name = "";
            else
                instance_name = "";
        }
    }

    int class_len = strlen(class_name);
    int instance_len = strlen(instance_name);
    char *result = malloc((class_len + instance_len + 2) * sizeof(*result));

    strcpy(result, instance_name);
    strcpy(result + instance_len + 1, class_name);
    *len = class_len + instance_len + 2;

    if (have_reply)
        xcb_icccm_get_wm_class_reply_wipe(&r);

    return result;
}

int b_prop_adjust(bonk_state_t *b)
{
    int wait = 0;
    int flags = 0;
    char *a_class = NULL, *a_instance = NULL, *a_title = NULL;

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            case opt_class:
                flags |= ADJUST_CLASS;
                a_class = optarg;
                break;
            case opt_instance:
                flags |= ADJUST_INSTANCE;
                a_instance = optarg;
                break;
            case opt_title:
                flags |= ADJUST_TITLE;
                a_title = optarg;
                break;
            case opt_wait:
                wait = 1;
                break;
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_only(b);

    BONK_FOREACH_WINDOW_DO(
        if (flags & (ADJUST_CLASS | ADJUST_INSTANCE)) {
            int buffer_len;
            char *buffer = get_wm_class_and_instance(
                    b, iter_window, a_class, a_instance, &buffer_len);

            xcb_icccm_set_wm_class(b->conn, iter_window, buffer_len, buffer);
            free(buffer);
        }
        if (flags & ADJUST_TITLE) {
            xcb_ewmh_set_wm_name(
                    b->ewmh, iter_window, strlen(a_title), a_title);
        }
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
