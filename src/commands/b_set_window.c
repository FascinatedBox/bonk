#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include <xcb/xcb_icccm.h>

#include "bonk_internal.h"

typedef enum {
    opt_class,
    opt_delete,
    opt_desktop,
    opt_help,
    opt_instance,
    opt_machine,
    opt_title,
    opt_wait,
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "class", required_argument, NULL, opt_class },
    { "delete", required_argument, NULL, opt_delete },
    { "desktop", required_argument, NULL, opt_desktop },
    { "help", no_argument, NULL, opt_help },
    { "instance", required_argument, NULL, opt_instance },
    { "machine", required_argument, NULL, opt_machine },
    { "title", required_argument, NULL, opt_title },
    { "wait", no_argument, NULL, opt_wait },
    { "window", required_argument, NULL, opt_window },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0]\n"
    "\n"
    "Adjust simple window properties.\n"
    "\n"
    "--class <name>           Update the class in WM_CLASS\n"
    "--delete <prop>          Remove property named <prop>\n"
    "--desktop <id>           The desktop the window is on (_NET_WM_DESKTOP)\n"
    "                         (first = 0, -1 = all desktops)\n"
    "--instance <name>        Update the instance in WM_CLASS\n"
    "--machine <name>         Update the name in WM_CLIENT_MACHINE\n"
    "--title <name>           Update the window's title (_NET_WM_NAME)\n"
    "\n"
    "--wait                   flush output buffer before next command\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    ;

#define ADJUST_CLASS    (1 << 0)
#define ADJUST_DESKTOP  (1 << 1)
#define ADJUST_INSTANCE (1 << 2)
#define ADJUST_MACHINE  (1 << 3)
#define ADJUST_TITLE    (1 << 4)

static char *get_wm_class_and_instance(bonk_state_t *b,
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

static int parse_desktop_id(bonk_state_t *b, const char *arg, long *value)
{
    char *end;
    long v = strtol(arg, &end, 10);

    if (v < -1 || *end != '\0') {
        fprintf(stderr, "bonk set-window error: Invalid desktop index '%s'.\n",
                arg);
        return 0;
    }

    *value = v;
    return 1;
}

/* Good enough for reasonable usage. */
#define MAX_AT_ONCE 8

int b_set_window(bonk_state_t *b)
{
    int wait = 0;
    int flags = 0;
    long a_desktop = 0;
    char *a_class = NULL, *a_instance = NULL, *a_machine = NULL,
         *a_title = NULL;
    xcb_atom_t a_to_delete[MAX_AT_ONCE];
    int delete_count = 0;

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            case opt_class:
                flags |= ADJUST_CLASS;
                a_class = optarg;
                break;
            case opt_delete: {
                if (delete_count == MAX_AT_ONCE) {
                    fprintf(stderr, "bonk set-window error: Too many atoms to delete at once (max = %d)\n",
                            MAX_AT_ONCE);
                    return 0;
                }

                /* Don't skip invalid atoms, or the limit will appear to be
                   enforced inconsistenly. */
                a_to_delete[delete_count] = bonk_atom_find_existing(b->conn,
                        optarg);
                delete_count++;
                break;
            }
            case opt_desktop:
                flags |= ADJUST_DESKTOP;

                if (parse_desktop_id(b, optarg, &a_desktop) == 0)
                    return 0;

                break;
            case opt_instance:
                flags |= ADJUST_INSTANCE;
                a_instance = optarg;
                break;
            case opt_machine:
                flags |= ADJUST_MACHINE;
                a_machine = optarg;
                break;
            case opt_title:
                flags |= ADJUST_TITLE;
                a_title = optarg;
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
        if (flags & ADJUST_MACHINE) {
            xcb_icccm_set_wm_client_machine(
                    b->conn, iter_window, b->ewmh->UTF8_STRING, 8,
                    strlen(a_machine), a_machine);
        }
        if (flags & ADJUST_TITLE) {
            xcb_ewmh_set_wm_name(
                    b->ewmh, iter_window, strlen(a_title), a_title);
        }
        if (flags & ADJUST_DESKTOP) {
            xcb_change_property(b->conn,
                                XCB_PROP_MODE_REPLACE,
                                iter_window,
                                b->ewmh->_NET_WM_DESKTOP,
                                XCB_ATOM_CARDINAL,
                                32,
                                1,
                                (const void *)&a_desktop);
        }
        if (delete_count) {
            for (int i = 0;i < delete_count;i++) {
                xcb_atom_t target = a_to_delete[i];

                if (target == XCB_ATOM_NONE)
                    continue;

                xcb_delete_property(b->conn, iter_window, target);
            }
        }
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
