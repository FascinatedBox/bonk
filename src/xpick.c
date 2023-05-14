#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/cursorfont.h>

#include "xpick.h"

typedef struct xpick_state_t {
    xcb_connection_t *conn;
    xcb_atom_t atom_wm_state;
    xcb_window_t root;
    xcb_window_t pick_w;
    bool pointer_grabbed;
} xpick_state_t;

static void intern_wm_state_atom(xpick_state_t *s)
 {
    const char *name = "WM_STATE";
    xcb_intern_atom_cookie_t c = xcb_intern_atom(
            s->conn,
            /* Only return an existing atom. */
            false,
            strlen(name),
            name);

    xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(
            s->conn, c, NULL);

    if (r == NULL)
        s->atom_wm_state = XCB_WINDOW_NONE;
    else {
        s->atom_wm_state = r->atom;
        free(r);
    }
}

xpick_state_t *xpick_state_new(xcb_connection_t *conn)
{
    xpick_state_t *s = malloc(sizeof(*s));

    s->conn = conn;
    intern_wm_state_atom(s);
    return s;
}

void xpick_state_free(xpick_state_t *s)
{
    free(s);
}

static xcb_cursor_t make_font_cursor(xpick_state_t *s)
{
    xcb_font_t cursor_font = xcb_generate_id(s->conn);
    xcb_cursor_t cursor = xcb_generate_id(s->conn);
    uint16_t cursor_glyph = XC_tcross;

    xcb_open_font(s->conn, cursor_font, strlen("cursor"), "cursor");
    xcb_create_glyph_cursor(s->conn,
                            cursor,
                            /* Source and mask (background) fonts to use. */
                            cursor_font,
                            cursor_font,
                            /* Same, but this time for the gylph. */
                            cursor_glyph,
                            cursor_glyph + 1,
                            /* Source and mask R, G, and B. */
                            0, 0, 0,
                            ~0, ~0, ~0);

    return cursor;
}

static xcb_window_t root_for_screen(xpick_state_t *s, uint16_t screen_index)
{
    const xcb_setup_t *setup = xcb_get_setup(s->conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    if (screen_index >= iter.rem)
        return XCB_WINDOW_NONE;

    for (int i = 0;i < screen_index;i++)
        xcb_screen_next(&iter);

    return iter.data->root;
}

bool xpick_cursor_grab(xpick_state_t *s, uint16_t screen_index)
{
    s->root = root_for_screen(s, screen_index);

    if (s->root == XCB_WINDOW_NONE)
        return false;

    xcb_cursor_t cursor = make_font_cursor(s);
    xcb_grab_pointer_cookie_t grab_cookie = xcb_grab_pointer(
            s->conn,
            false, /* Don't send the click to the underlying client. */
            s->root, /* Grab root window (everything). */

            /* Listen for both a press and the release. This way, unless a user
               presses a cancel button, the underlying application won't receive
               a release for this client's press. */
            XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_BUTTON_RELEASE,
            XCB_GRAB_MODE_SYNC,
            XCB_GRAB_MODE_ASYNC,
            s->root, /* Pointer must stay here. */
            cursor, /* Show this cursor. */
            XCB_TIME_CURRENT_TIME);

    xcb_grab_pointer_reply_t *reply = xcb_grab_pointer_reply(
            s->conn,
            grab_cookie,
            NULL);

    if (reply != NULL) {
        s->pointer_grabbed = reply->status == XCB_GRAB_STATUS_SUCCESS;
        free(reply);
    }
    else
        s->pointer_grabbed = false;

    return s->pointer_grabbed;
}

bool xpick_cursor_pick_window(xpick_state_t *s)
{
    s->pick_w = XCB_WINDOW_NONE;

    if (s->pointer_grabbed == false)
        return false;

    xcb_window_t pick_w = XCB_WINDOW_NONE;

    for (;;) {
        xcb_allow_events(
                s->conn, XCB_ALLOW_SYNC_POINTER, XCB_TIME_CURRENT_TIME);
        xcb_flush(s->conn);
        xcb_generic_event_t *event = xcb_wait_for_event(s->conn);

        if (event == NULL)
            break;

        uint8_t response = event->response_type & 0x7f;

        /* Some events are sent to clients even if they're not included in the
           event mask. Ignore them. */
        if (response != XCB_BUTTON_PRESS &&
            response != XCB_BUTTON_RELEASE) {
            free(event);
            continue;
        }

        xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;

        if (response == XCB_BUTTON_PRESS) {
            /* Detail is the button pressed (left is 1). Treat any non-left
               mouse press as a cancel and give up. */
            if (bp->detail != 1) {
                free(event);
                break;
            }

            if (bp->child != XCB_WINDOW_NONE)
                pick_w = bp->child;
            else
                /* It's the root window. */
                pick_w = bp->root;

            free(event);
        }
        else {
            s->pick_w = pick_w;
            free(event);
            break;
        }
    }

    return s->pick_w != XCB_WINDOW_NONE;
}

void xpick_cursor_ungrab(xpick_state_t *s)
{
    if (s->pointer_grabbed == false)
        return;

    xcb_ungrab_pointer(s->conn, XCB_TIME_CURRENT_TIME);
    s->pointer_grabbed = false;
}

static bool window_has_wm_state(xpick_state_t *s, xcb_window_t window)
{
    xcb_get_property_cookie_t c = xcb_get_property(
            s->conn,
            false, /* Do not delete this property. */
            window,
            s->atom_wm_state,
            /* Not interested in the type or the data inside. */
            XCB_GET_PROPERTY_TYPE_ANY, 0, 0);

    xcb_get_property_reply_t *r = xcb_get_property_reply(s->conn, c, NULL);

    if (r == NULL)
        return false;

    bool result = r->type != XCB_NONE;

    free(r);
    return result;
}

static xcb_window_t find_wm_window_within(
        xpick_state_t *s,
        xcb_window_t source_w)
{
    xcb_query_tree_cookie_t c = xcb_query_tree(s->conn, source_w);
    bool has_wm_state = window_has_wm_state(s, source_w);
    xcb_query_tree_reply_t *r = xcb_query_tree_reply(s->conn, c, NULL);

    if (has_wm_state) {
        free(r);
        return source_w;
    }

    if (r == NULL)
        return XCB_WINDOW_NONE;

    int child_count = xcb_query_tree_children_length(r);
    xcb_window_t *children = xcb_query_tree_children(r);
    xcb_window_t result = XCB_WINDOW_NONE;

    for (int i = 0;i < child_count;i++) {
        result = find_wm_window_within(s, children[i]);

        if (result != XCB_WINDOW_NONE)
            break;
    }

    free(r);
    return result;
}

xcb_window_t xpick_window_get(xpick_state_t *s)
{
    if (s->pick_w == XCB_WINDOW_NONE)
        return XCB_WINDOW_NONE;

    xcb_window_t result = s->pick_w;
    xcb_window_t window = find_wm_window_within(s, s->pick_w);

    if (window != XCB_WINDOW_NONE)
        result = window;

    return result;
}
