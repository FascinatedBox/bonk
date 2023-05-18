#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "bonk_internal.h"

bonk_state_t *bonk_new_state(int argc_, char **argv_)
{
    bonk_state_t *b = malloc(sizeof(*b));

    b->conn = xcb_connect(NULL, NULL);
    b->argc = argc_;
    b->argv = argv_;
    b->ewmh = malloc(sizeof(*b->ewmh));
    b->window_stack = bonk_new_window_list(1);
    b->current_command = NULL;

    if (xcb_connection_has_error(b->conn)) {
        fprintf(stderr, "bonk error: Unable to open X display. Stopping.\n");
        exit(EXIT_FAILURE);
    }

    xcb_intern_atom_cookie_t *c = xcb_ewmh_init_atoms(b->conn, b->ewmh);

    xcb_ewmh_init_atoms_replies(b->ewmh, c, NULL);
    return b;
}

void bonk_free_state(bonk_state_t *b)
{
    xcb_connection_t *conn = b->conn;

    /* Do *not* replace this line with xcb_flush, they are *not* equivalent.
       Simply flushing the output alone does not guarantee that the output has
       been processed.
       To make sure that *all* requests are processed, this sends a focus
       request and waits for the reply to it.
       Not doing this will result in the requests this tool sends only
       occasionally working. */
    bonk_connection_flush(b);

    bonk_free_window_list(b->window_stack);
    xcb_ewmh_connection_wipe(b->ewmh);
    free(b->ewmh);
    xcb_disconnect(conn);
    free(b);
}

typedef int (*bonk_func)(bonk_state_t *);

typedef struct {
    char *name;
    bonk_func dispatch_func;
} bonk_dispatch_entry;

extern int b_activate(bonk_state_t *);
extern int b_close(bonk_state_t *);
extern int b_decoration(bonk_state_t *);
extern int b_focus(bonk_state_t *);
extern int b_get_active(bonk_state_t *);
extern int b_get_focus(bonk_state_t *);
extern int b_get_title(bonk_state_t *);
extern int b_help(bonk_state_t *);
extern int b_lower(bonk_state_t *);
extern int b_map(bonk_state_t *);
extern int b_move_resize(bonk_state_t *);
extern int b_pick(bonk_state_t *);
extern int b_prop_adjust(bonk_state_t *);
extern int b_prop_delete(bonk_state_t *);
extern int b_raise(bonk_state_t *);
extern int b_select(bonk_state_t *);
extern int b_state(bonk_state_t *);
extern int b_unmap(bonk_state_t *);

bonk_dispatch_entry dispatch_table[] = {
    { "activate",             b_activate },
    { "close",                b_close },
    { "decoration",           b_decoration },
    { "focus",                b_focus },
    { "get-active",           b_get_active },
    { "get-focus",            b_get_focus },
    { "get-title",            b_get_title },
    { "help",                 b_help },
    { "lower",                b_lower },
    { "map",                  b_map },
    { "move-resize",          b_move_resize },
    { "pick",                 b_pick },
    { "prop-adjust",          b_prop_adjust },
    { "prop-delete",          b_prop_delete },
    { "raise",                b_raise },
    { "select",               b_select },
    { "state",                b_state },
    { "unmap",                b_unmap },
    { NULL,                   NULL },
};

int bonk_exec(bonk_state_t *b)
{
    int result = 1;

    b->argv++;
    b->argc--;
    optind = 0;

    while (b->argc) {
        char *command = b->argv[0];
        int found = 0;
        int i;
        bonk_dispatch_entry e = dispatch_table[0];

        for (i = 0;e.name != NULL;i++, e = dispatch_table[i]) {
            if (e.name[0] != command[0] ||
                strcmp(e.name, command) != 0)
                continue;

            b->current_command = command;
            found = 1;
            result = e.dispatch_func(b);
            break;
        }

        if (found == 0) {
            fprintf(stderr, "bonk error: Invalid command '%s'.\n", command);
            result = 0;
        }

        if (result == 0)
            break;
    }

    return result == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    bonk_state_t *b = bonk_new_state(argc, argv);
    int result = bonk_exec(b);

    bonk_free_state(b);

    return result;
}
