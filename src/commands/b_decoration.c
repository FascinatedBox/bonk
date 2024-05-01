#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "bonk_internal.h"

/* Mostly unused, but added for completeness. */
#define MWM_FUNC_ALL       (1L << 0)
#define MWM_FUNC_RESIZE    (1L << 1)
#define MWM_FUNC_MOVE      (1L << 2)
#define MWM_FUNC_MINIMIZE  (1L << 3)
#define MWM_FUNC_MAXIMIZE  (1L << 4)
#define MWM_FUNC_CLOSE     (1L << 5)

#define MWM_DECOR_ALL      (1L << 0)
#define MWM_DECOR_BORDER   (1L << 1)
#define MWM_DECOR_RESIZEH  (1L << 2)
#define MWM_DECOR_TITLE    (1L << 3)
#define MWM_DECOR_MENU     (1L << 4)
#define MWM_DECOR_MINIMIZE (1L << 5)
#define MWM_DECOR_MAXIMIZE (1L << 6)

#define HINT_FUNCTIONS (1L << 0)
#define HINT_DECORATIONS (1L << 1)

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
    "   all, none, border, close, maximize, minimize, resize, title\n"
    "\n"
    "Decorations must be given as a single comma-separated string:\n"
    "   decoration resize,close\n"
    ;

typedef struct {
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    uint32_t input_mode;
    uint32_t status;
} bonk_motif_wm_hints;

static void do_set_motif_wm_hints(bonk_state_t *b,
                                  xcb_window_t window,
                                  xcb_atom_t motif_atom,
                                  uint32_t functions,
                                  uint32_t decorations)
{
    bonk_motif_wm_hints hints;

    memset(&hints, 0, sizeof(hints));

    hints.flags = HINT_DECORATIONS;
    hints.decorations = decorations;

    if (functions != UINT32_MAX) {
        hints.flags |= HINT_FUNCTIONS;
        hints.functions = functions;
    }

    xcb_change_property(b->conn,
                        XCB_PROP_MODE_REPLACE,
                        window,
                        motif_atom,
                        motif_atom,
                        32,
                        5,
                        (char *)&hints);
}

static uint32_t parse_decorations(char *decoration_str,
                                  uint32_t *functions,
                                  uint32_t *decorations)
{
    char *tok, *save_ptr;
    uint32_t d = 0, funcs = UINT32_MAX;

    while ((tok = strtok_r(decoration_str, ",", &save_ptr))) {
        if (decoration_str != NULL)
            decoration_str = NULL;

        if (strcmp(tok, "none") == 0)
            d = 0;
        else if (strcmp(tok, "border") == 0)
            d |= MWM_DECOR_BORDER;
        else if (strcmp(tok, "maximize") == 0)
            d |= MWM_DECOR_MAXIMIZE;
        else if (strcmp(tok, "menu") == 0)
            d |= MWM_DECOR_MENU;
        else if (strcmp(tok, "minimize") == 0)
            d |= MWM_DECOR_MINIMIZE;
        else if (strcmp(tok, "resize") == 0)
            d |= MWM_DECOR_RESIZEH;
        else if (strcmp(tok, "title") == 0)
            d |= MWM_DECOR_TITLE;
        else if (strcmp(tok, "close") == 0)
            funcs = MWM_FUNC_ALL | MWM_FUNC_CLOSE;
        else if (strcmp(tok, "all") == 0)
            d = MWM_DECOR_ALL;
        else {
            fprintf(stderr, "bonk decoration error: Invalid decoration '%s'.\n", tok);
            d = UINT32_MAX;
            break;
        }
    }

    if (funcs == UINT32_MAX)
        funcs = MWM_FUNC_ALL;

    *decorations = d;
    *functions = funcs;
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

    uint32_t funcs, decorations;
    parse_decorations(bonk_arg_next_unchecked(b), &funcs, &decorations);

    xcb_atom_t motif_atom = bonk_atom_find_or_intern(b->conn,
            "_MOTIF_WM_HINTS");

    if (decorations == UINT32_MAX || motif_atom == 0)
        return 0;

    BONK_FOREACH_WINDOW_DO(
        do_set_motif_wm_hints(b, iter_window, motif_atom, funcs, decorations);
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
