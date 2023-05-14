#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "bonk_internal.h"

extern void bonk_free_state(bonk_state_t *);

void bonk_usage(bonk_state_t *b, const char *usage)
{
    fprintf(stderr, usage, b->argv[0]);
    bonk_free_state(b);
    exit(EXIT_SUCCESS);
}

char *bonk_arg_next_unchecked(bonk_state_t *b)
{
    char *arg = b->argv[0];

    b->argv++;
    b->argc--;

    return arg;
}

static void getopt_advance(bonk_state_t *b)
{
    b->argv += optind;
    b->argc -= optind;

    optind = 0;
}

void bonk_arg_require_n(bonk_state_t *b, int n)
{
    getopt_advance(b);

    if (b->argc >= n)
        return;

    fprintf(stderr,
            "bonk %s error: Command expected %d args, but only have %d left. Exiting.\n",
            b->current_command, n, b->argc);

    /* Free the state before stopping to make sure all requests land. */
    bonk_free_state(b);
    exit(EXIT_FAILURE);
}

static char *get_arg_if_maybe_window_arg(bonk_state_t *b)
{
    if (b->window_stack->pos == 0) {
        fprintf(stderr, "bonk %s error: The window stack is empty. Stopping.\n",
                b->current_command);
        exit(EXIT_FAILURE);
    }

    if (b->argc == 0)
        return NULL;

    char *arg = b->argv[0];

    if (arg[0] != '%')
        return NULL;

    b->argv++;
    b->argc--;

    /* Skip the percent so window argument parsing doesn't have to. */
    return arg + 1;
}

static void parse_window_arg(bonk_state_t *b, char *arg)
{
    if (arg == NULL) {
        /* No explicit window arg, so default to the first window provided.
           Arg checking confirms that there is a window, so this is safe. */
        b->iter_windows = &b->window_stack->data[0];
        b->iter_window_count = 1;
        return;
    }

    /* Special case %@ to mean "all windows" since the default is the first one
       in the stack. */
    if (arg[0] == '@' && arg[1] == '\0') {
        b->iter_windows = b->window_stack->data;
        b->iter_window_count = b->window_stack->pos;
        return;
    }

    int index = atoi(arg);

    if (index < 0)
        index += (int)b->window_stack->pos;

    if (index >= b->window_stack->pos ||
        index < 0 ||
        arg[0] == '\0') {
        fprintf(stderr, "bonk error: Invalid window index '%s' (%%%d .. %%%d).\n",
                arg, b->window_stack->pos - 1, -(int)b->window_stack->pos);
        exit(EXIT_FAILURE);
    }

    b->iter_windows = &b->window_stack->data[index];
    b->iter_window_count = 1;
}

void bonk_arg_window_and_require_n(bonk_state_t *b, int n)
{
    getopt_advance(b);
    parse_window_arg(b, get_arg_if_maybe_window_arg(b));

    bonk_arg_require_n(b, n);
}

void bonk_arg_window_only(bonk_state_t *b)
{
    getopt_advance(b);
    parse_window_arg(b, get_arg_if_maybe_window_arg(b));
}

static xcb_atom_t _string_to_atom(xcb_connection_t *conn,
                                  const char *text,
                                  int must_exist)
{
    xcb_intern_atom_cookie_t c = xcb_intern_atom(
            conn,
            must_exist,
            strlen(text),
            text);
    xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(conn, c, NULL);
    xcb_window_t result = XCB_WINDOW_NONE;

    if (r) {
        result = r->atom;
        free(r);
    }

    return result;
}

xcb_atom_t bonk_atom_find_existing(xcb_connection_t *conn, const char *text)
{
    return _string_to_atom(conn, text, 1);
}

xcb_atom_t bonk_atom_find_or_intern(xcb_connection_t *conn, const char *text)
{
    return _string_to_atom(conn, text, 0);
}

void bonk_connection_flush(bonk_state_t *b)
{
    xcb_get_input_focus_cookie_t c = xcb_get_input_focus(b->conn);
    xcb_get_input_focus_reply_t *r = xcb_get_input_focus_reply(b->conn, c, NULL);

    free(r);
}

xcb_window_t *bonk_window_list_get(bonk_state_t *b, int *count)
{
    *count = b->iter_window_count;

    return b->iter_windows;
}

void bonk_use_window_arg(bonk_state_t *b, const char *arg)
{
    /* The window stack should always exist and always have at least one space
       reserved. Since the user is manually specifying a window, assume they
       know what they're doing and that the window is valid. If the user is
       wrong and the window is bad, they'll get an X error when trying to use
       the window. */
    b->window_stack->data[0] = strtol(arg, NULL, 0);
    b->window_stack->pos = 1;
}

void bonk_strcpy_upper(char *a, const char *b)
{
    while (*b) {
        *a = toupper(*b);
        a++;
        b++;
    }

    *a = '\0';
}
