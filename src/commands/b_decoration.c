#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "bonk_internal.h"

#define DECORATION_ALL (1L << 0)
#define DECORATION_RESIZE (1L << 1)
#define DECORATION_MOVE (1L << 2)
#define DECORATION_MINIMIZE (1L << 3)
#define DECORATION_MAXIMIZE (1L << 4)
#define DECORATION_CLOSE (1L << 5)

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
    "Usage: %s [<window-arg>=%0] decorations...\n"
    "\n"
    "Adjust a window's decorations (_MOTIF_WM_HINTS)\n"
    "\n"
    "--wait                   flush output buffer before next command\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    "-h, --help               display this help and exit\n"
    "\n"
    "Set a window's decorations to only include 'decorations'.\n"
    "Valid decorations are:\n"
    "   all, none, close, maximize, minimize, move, resize\n"
    "\n"
    "Decorations must be given as a single comma-separated string:\n"
    "   decoration resize,move,close\n"
    ;

/* I have no idea what's going here, I just know enough to get motif to adjust
   window decorations. */
typedef struct {
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    uint32_t input_mode;
    uint32_t status;
} bonk_motif_wm_hints;

#define HINT_FUNCTIONS (1L << 0)
#define HINT_DECORATIONS (1L << 1)

static void do_set_motif_wm_hints(bonk_state_t *b,
                                  xcb_window_t window,
                                  xcb_atom_t motif_atom,
                                  uint32_t decorations)
{
    bonk_motif_wm_hints hints;

    memset(&hints, 0, sizeof(hints));
    hints.flags = HINT_FUNCTIONS | HINT_DECORATIONS;
    hints.decorations = decorations;
    hints.functions = decorations;

    xcb_change_property(b->conn,
                        XCB_PROP_MODE_REPLACE,
                        window,
                        motif_atom,
                        motif_atom,
                        32,
                        5,
                        (char *)&hints);
}

static uint32_t parse_decorations(char *decoration_str)
{
    char *tok, *save_ptr;
    int decorations = 0;

    while ((tok = strtok_r(decoration_str, ",", &save_ptr))) {
        if (decoration_str != NULL)
            decoration_str = NULL;

        if (strcmp(tok, "none") == 0)
            decorations = 0;
        else if (strcmp(tok, "resize") == 0)
            decorations |= DECORATION_RESIZE;
        else if (strcmp(tok, "minimize") == 0)
            decorations |= DECORATION_RESIZE | DECORATION_MINIMIZE;
        else if (strcmp(tok, "maximize") == 0)
            decorations |= DECORATION_RESIZE | DECORATION_MAXIMIZE;
        else if (strcmp(tok, "close") == 0)
            decorations |= DECORATION_RESIZE | DECORATION_CLOSE;
        else if (strcmp(tok, "all") == 0)
            decorations = DECORATION_RESIZE |
                          DECORATION_MOVE |
                          DECORATION_MAXIMIZE |
                          DECORATION_MINIMIZE |
                          DECORATION_CLOSE;
        else {
            fprintf(stderr, "bonk decoration error: Invalid decoration '%s'.\n", tok);
            return UINT32_MAX;
        }
    }

    return decorations;
}

int b_decoration(bonk_state_t *b)
{
    int wait = 0;

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_and_require_n(b, 1);

    uint32_t decorations = parse_decorations(bonk_arg_next_unchecked(b));
    xcb_atom_t motif_atom = bonk_atom_find_or_intern(b->conn,
            "_MOTIF_WM_HINTS");

    if (decorations == UINT32_MAX || motif_atom == 0)
        return 0;

    BONK_FOREACH_WINDOW_DO(
        do_set_motif_wm_hints(b, iter_window, motif_atom, decorations);
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
