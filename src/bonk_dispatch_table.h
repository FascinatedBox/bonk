#ifndef BONK_DISPATCH_TABLE_H
# define BONK_DISPATCH_TABLE_H

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
extern int b_opacity(bonk_state_t *);
extern int b_pick(bonk_state_t *);
extern int b_raise(bonk_state_t *);
extern int b_reject(bonk_state_t *);
extern int b_select(bonk_state_t *);
extern int b_set_window(bonk_state_t *);
extern int b_sleep(bonk_state_t *);
extern int b_state(bonk_state_t *);
extern int b_terminate(bonk_state_t *);
extern int b_unmap(bonk_state_t *);

static bonk_dispatch_entry dispatch_table[] = {
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
    { "opacity",              b_opacity },
    { "pick",                 b_pick },
    { "raise",                b_raise },
    { "reject",               b_reject },
    { "select",               b_select },
    { "set-window",           b_set_window },
    { "sleep",                b_sleep },
    { "state",                b_state },
    { "terminate",            b_terminate },
    { "unmap",                b_unmap },
    { NULL,                   NULL },
};

#endif
