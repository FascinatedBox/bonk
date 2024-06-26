#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
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

char *bonk_arg_take_next(bonk_state_t *b)
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

static void pick_first_window(bonk_state_t *b)
{
    if (b->window_stack->pos == 0) {
        fprintf(stderr, "bonk %s error: The window stack is empty. Stopping.\n",
                b->current_command);
        exit(EXIT_FAILURE);
    }

    b->iter_windows = &b->window_stack->data[0];
    b->iter_window_count = 1;
}

static void parse_percent_arg(bonk_state_t *b, const char *arg_start)
{
    const char *arg = arg_start;
    int digits = 0, is_negative = 0, ok = 0, total = 0;

    if (b->window_stack->pos == 0) {
        fprintf(stderr, "bonk %s error: The window stack is empty. Stopping.\n",
                b->current_command);
        exit(EXIT_FAILURE);
    }

    if (*arg == '-') {
        is_negative = 1;
        arg++;
    }

    /* Keep it simple: 0...999, no leading zero trimming. */
    for (digits = 0;digits < 4;digits++) {
        if (*arg == '\0') {
            /* Block '%' and '%-'. */
            ok = digits;
            break;
        }

        if (*arg < '0' || *arg > '9') {
            ok = 0;
            break;
        }

        total = (total * 10) + (*arg - '0');
        arg++;
    }

    if (is_negative)
        total = -total + (int)b->window_stack->pos;

    if (total >= b->window_stack->pos ||
        total < 0 ||
        ok == 0) {
        fprintf(stderr, "bonk error: Invalid window index '%s' (%%%d .. %%%d).\n",
                arg_start, b->window_stack->pos - 1,
                -(int)b->window_stack->pos);
        exit(EXIT_FAILURE);
    }

    b->iter_windows = &b->window_stack->data[total];
    b->iter_window_count = 1;
}

static void setup_iter_windows(bonk_state_t *b)
{
    if (b->argc == 0) {
        pick_first_window(b);
        return;
    }

    char *arg = b->argv[0];

    if (arg[0] != '%') {
        /* Argument is not a window, so do not consume. */
        pick_first_window(b);
        return;
    }

    b->argv++;
    b->argc--;
    /* Skip past starting '%'. */
    arg = arg + 1;

    if (arg[0] == '@' && arg[1] == '\0') {
        /* For %@, send all windows. */
        b->iter_windows = b->window_stack->data;
        b->iter_window_count = b->window_stack->pos;
        return;
    }

    parse_percent_arg(b, arg);
}

void bonk_arg_window_and_require_n(bonk_state_t *b, int n)
{
    getopt_advance(b);
    setup_iter_windows(b);
    bonk_arg_require_n(b, n);
}

void bonk_arg_window_only(bonk_state_t *b)
{
    getopt_advance(b);
    setup_iter_windows(b);
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

static void strcpy_upper(char *a, const char *b)
{
    while (*b) {
        *a = toupper(*b);
        a++;
        b++;
    }

    *a = '\0';
}

static int find_state_atom_index(const char *str)
{
    static char *atom_table[] = {
        "ABOVE",
        "BELOW",
        "DEMANDS_ATTENTION",
        "FULLSCREEN",
        "HIDDEN",
        "MAXIMIZED_HORZ",
        "MAXIMIZED_VERT",
        "MODAL",
        "SHADED",
        "SKIP_PAGER",
        "SKIP_TASKBAR",
        "STICKY",
        NULL,
    };
    char *atom_name, upper_buffer[64];
    int i;

    strcpy_upper(upper_buffer, str);

    for (i = 0, atom_name = atom_table[i];
         atom_name != NULL;
         i++, atom_name = atom_table[i]) {

        if (atom_name[0] != upper_buffer[0] ||
            strcmp(atom_name, upper_buffer) != 0)
            continue;

        break;
    }

    return i;
}

#define B_ABOVE              0
#define B_BELOW              1
#define B_DEMANDS_ATTENTION  2
#define B_FULLSCREEN         3
#define B_HIDDEN             4
#define B_MAXIMIZED_HORZ     5
#define B_MAXIMIZED_VERT     6
#define B_MODAL              7
#define B_SHADED             8
#define B_SKIP_PAGER         9
#define B_SKIP_TASKBAR      10
#define B_STICKY            11

#define ATOM_CASE(index_name) \
case B_##index_name: \
    result = ewmh->_NET_WM_STATE_##index_name; \
    break;

static xcb_atom_t window_state_atom_from_index(bonk_state_t *b, int index)
{
    xcb_ewmh_connection_t *ewmh = b->ewmh;
    xcb_atom_t result;

    switch (index) {
        ATOM_CASE(ABOVE)
        ATOM_CASE(BELOW)
        ATOM_CASE(DEMANDS_ATTENTION)
        ATOM_CASE(FULLSCREEN)
        ATOM_CASE(HIDDEN)
        ATOM_CASE(MAXIMIZED_HORZ)
        ATOM_CASE(MAXIMIZED_VERT)
        ATOM_CASE(MODAL)
        ATOM_CASE(SHADED)
        ATOM_CASE(SKIP_PAGER)
        ATOM_CASE(SKIP_TASKBAR)
        ATOM_CASE(STICKY)
        default:
            result = XCB_ATOM_NONE;
            break;
    }

    return result;
}

xcb_atom_t bonk_window_state_atom_from_string(bonk_state_t *b, const char *str)
{
    if (strlen(str) >= 32)
        return XCB_ATOM_NONE;

    int index = find_state_atom_index(str);
    xcb_atom_t result = window_state_atom_from_index(b, index);

    return result;
}

static void do_exec_man(char *buffer)
{
    execlp("man", "man", buffer, (char *)NULL);
    fputs("bonk error: Failed to exec 'man' to view help.", stderr);
    exit(EXIT_FAILURE);
}

void bonk_exec_command_man(bonk_state_t *b)
{
    char buffer[128] = "bonk-";

    strcat(buffer, b->current_command);
    do_exec_man(buffer);
}

void bonk_exec_man_for(const char *page)
{
    char *buffer = malloc(strlen(page) + strlen("bonk-") + 1);

    strcpy(buffer, "bonk-");
    strcat(buffer, page);
    do_exec_man(buffer);
}
