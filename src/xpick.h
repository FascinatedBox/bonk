#ifndef XPICK_H
# define XPICK_H
# include <stdbool.h>

# include <xcb/xcb.h>
# include <xcb/xproto.h>

typedef struct xpick_state_t xpick_state_t;

xpick_state_t *xpick_state_new(xcb_connection_t *connection);
void xpick_state_free(xpick_state_t *s);

bool xpick_cursor_grab(xpick_state_t *s, uint16_t screen_index);
bool xpick_cursor_pick_window(xpick_state_t *s);
void xpick_cursor_ungrab(xpick_state_t *s);

xcb_window_t xpick_window_get(xpick_state_t *s);

#endif
