#include <string.h>

#include <xcb/xcb_icccm.h>

#include "bonk_select.h"

#define T_MAX_TITLE 512
#define DATA_TO_TITLE_BUFFER(field) \
int len = data.field##_len; \
\
if (len > T_MAX_TITLE) \
    len = T_MAX_TITLE; \
\
strncpy(buffer, data.field, len); \
buffer[len] = '\0';

static int match_criteria(bonk_select_t *s,
                          bonk_criteria criteria,
                          const char *text,
                          int flag)
{
    int result;

    if (s->rxmask & flag)
        result = regexec(&criteria.pattern, text, 0, NULL, 0);
    else if (s->mask)
        result = strcmp(criteria.text, text);

    return !result;
}

static void request_ewmh_names(bonk_select_t *s)
{
    int i;

    for (i = 0;i < s->window_stack->pos;i++) {
        xcb_window_t w = s->window_stack->data[i];
        xcb_get_property_cookie_t c = xcb_ewmh_get_wm_name(s->ewmh, w);

        bonk_cookie_push(s->jar, c);
    }
}

static void compare_ewmh_names(bonk_select_t *s, char *status_buffer)
{
    char buffer[T_MAX_TITLE];
    int i;

    BONK_INIT_COOKIE_LIST(xcb_get_property_cookie_t *, cookie_list);

    for (i = 0;i < s->jar->pos;i++) {
        xcb_get_property_cookie_t c = cookie_list[i];
        xcb_ewmh_get_utf8_strings_reply_t data;

        if (xcb_ewmh_get_wm_name_reply(s->ewmh, c, &data, NULL) == 0)
            continue;

        DATA_TO_TITLE_BUFFER(strings)

        int result = match_criteria(s, s->title_criteria, buffer,
                B_SELECT_TITLE);

        status_buffer[i] = result + 1;
        xcb_ewmh_get_utf8_strings_reply_wipe(&data);
    }

    s->jar->pos = 0;
}

static void request_icccm_names(bonk_select_t *s, char *status_buffer)
{
    int i;

    for (i = 0;i < s->window_stack->pos;i++) {
        if (status_buffer[i] != 0)
            continue;

        xcb_window_t w = s->window_stack->data[i];
        xcb_get_property_cookie_t c = xcb_ewmh_get_wm_name(s->ewmh, w);

        bonk_cookie_push(s->jar, c);
    }
}

static void compare_icccm_names(bonk_select_t *s, char *status_buffer)
{
    char buffer[T_MAX_TITLE];
    int window_i, cookie_j;

    BONK_INIT_COOKIE_LIST(xcb_get_property_cookie_t *, cookie_list);

    for (window_i = 0, cookie_j = 0;
         cookie_j < s->jar->pos;
         window_i++) {
        if (status_buffer[window_i] == 0)
            /* Already matched against this window. */
            continue;

        xcb_icccm_get_text_property_reply_t data;
        xcb_get_property_cookie_t c = cookie_list[cookie_j];

        cookie_j++;

        if (xcb_icccm_get_wm_name_reply(s->conn, c, &data, NULL) == 0)
            continue;

        DATA_TO_TITLE_BUFFER(name)

        int result = match_criteria(s, s->title_criteria, buffer,
                B_SELECT_TITLE);

        status_buffer[window_i] = result + 1;
        xcb_icccm_get_text_property_reply_wipe(&data);
    }

    s->jar->pos = 0;
}

static void filter_by_status_buffer(bonk_select_t *s, char *status_buffer)
{
    xcb_window_t *window_list = s->window_stack->data;
    int cursor, i;

    for (cursor = 0, i = 0;i < s->window_stack->pos;i++) {
        /* match_criteria will return 0 if not matched, 1 if matched and the
           buffer's values are 1 + match_criteria's output. */
        if (status_buffer[i] == 2) {
            window_list[cursor] = window_list[i];
            cursor++;
        }
    }

    s->window_stack->pos = cursor;
}

static void filter_title(bonk_select_t *s)
{
    char *status_buffer = calloc(s->window_stack->pos, sizeof(*status_buffer));

    request_ewmh_names(s);
    compare_ewmh_names(s, status_buffer);
    request_icccm_names(s, status_buffer);
    compare_icccm_names(s, status_buffer);
    filter_by_status_buffer(s, status_buffer);
    free(status_buffer);
}

static void filter_has_prop(bonk_select_t *s)
{
    int i;

    for (i = 0;i < s->window_stack->pos;i++) {
        xcb_window_t w = s->window_stack->data[i];
        xcb_get_property_cookie_t c = xcb_get_property(
                s->conn,
                0, /* Don't delete it */
                w,
                s->has_prop,
                XCB_GET_PROPERTY_TYPE_ANY,
                /* Start at 0 (beginning), and return 0 bytes. */
                0, 0);

        bonk_cookie_push(s->jar, c);
    }

    BONK_INIT_COOKIE_LIST(xcb_get_property_cookie_t *, cookie_list);
    xcb_window_t *window_list = s->window_stack->data;
    int cursor = 0;

    for (i = 0;i < s->jar->pos;i++) {
        xcb_get_property_cookie_t c = cookie_list[i];
        xcb_get_property_reply_t *r = xcb_get_property_reply(s->conn, c, NULL);

        if (r == NULL)
            continue;

        if (r->type == XCB_ATOM_NONE) {
            free(r);
            continue;
        }

        window_list[cursor] = window_list[i];
        cursor++;
        free(r);
    }

    s->window_stack->pos = cursor;
    s->jar->pos = 0;
}

static void filter_class_and_instance(bonk_select_t *s)
{
    int i;

    for (i = 0;i < s->window_stack->pos;i++) {
        xcb_window_t w = s->window_stack->data[i];
        xcb_get_property_cookie_t c = xcb_icccm_get_wm_class(s->conn, w);

        bonk_cookie_push(s->jar, c);
    }

    BONK_INIT_COOKIE_LIST(xcb_get_property_cookie_t *, cookie_list);
    xcb_window_t *window_list = s->window_stack->data;
    xcb_connection_t *conn = s->conn;
    int check_class = s->mask & B_SELECT_CLASS;
    int check_instance = s->mask & B_SELECT_INSTANCE;
    int cursor = 0;

    for (i = 0;i < s->jar->pos;i++) {
        xcb_get_property_cookie_t c = cookie_list[i];
        xcb_icccm_get_wm_class_reply_t r;

        if (xcb_icccm_get_wm_class_reply(conn, c, &r, NULL) == 0)
            continue;

        int result = 1;

        if (check_class)
            result = match_criteria(s, s->class_criteria, r.class_name,
                                    B_SELECT_CLASS);
        else
            result = 1;

        if (result && check_instance)
            result = match_criteria(s, s->instance_criteria, r.instance_name,
                                    B_SELECT_INSTANCE);

        if (result == 0) {
            xcb_icccm_get_wm_class_reply_wipe(&r);
            continue;
        }

        window_list[cursor] = window_list[i];
        cursor++;
        xcb_icccm_get_wm_class_reply_wipe(&r);
    }

    s->window_stack->pos = cursor;
    s->jar->pos = 0;
}

static void filter_visible(bonk_select_t *s)
{
    int i;

    for (i = 0;i < s->window_stack->pos;i++) {
        xcb_window_t w = s->window_stack->data[i];
        xcb_get_window_attributes_cookie_t c = xcb_get_window_attributes(s->conn, w);

        bonk_cookie_push(s->jar, c);
    }

    BONK_INIT_COOKIE_LIST(xcb_get_window_attributes_cookie_t *, cookie_list);
    xcb_connection_t *conn = s->conn;
    xcb_window_t *window_list = s->window_stack->data;
    int cursor = 0;

    for (i = 0;i < s->jar->pos;i++) {
        xcb_get_window_attributes_cookie_t c = cookie_list[i];
        xcb_get_window_attributes_reply_t *r =
                xcb_get_window_attributes_reply(conn, c, NULL);

        if (r == NULL)
            continue;

        if (r->map_state != XCB_MAP_STATE_VIEWABLE) {
            free(r);
            continue;
        }

        window_list[cursor] = window_list[i];
        cursor++;
        free(r);
    }

    s->window_stack->pos = cursor;
    s->jar->pos = 0;
}

static void walk_for_windows(bonk_select_t *s)
{
    xcb_connection_t *conn = s->conn;

    while (s->jar->pos) {
        BONK_INIT_COOKIE_LIST(xcb_query_tree_cookie_t *, cookie_list);
        int i;

        for (i = 0;i < s->jar->pos;i++) {
            xcb_query_tree_cookie_t c = cookie_list[i];

            xcb_query_tree_reply_t *reply =
                    xcb_query_tree_reply(conn, c, NULL);

            if (reply == NULL)
                continue;

            int c_len = xcb_query_tree_children_length(reply);
            xcb_window_t *c_list = xcb_query_tree_children(reply);

            int c_i;
            for (c_i = 0; c_i < c_len; c_i++) {
                xcb_window_t w = c_list[c_i];
                xcb_query_tree_cookie_t c2 = xcb_query_tree(conn, w);

                bonk_window_list_push(s->window_stack, w);
                bonk_cookie_push(s->aux_jar, c2);
            }

            free(reply);
        }

        s->jar->pos = 0;

        bonk_cookie_jar *temp = s->jar;
        s->jar = s->aux_jar;
        s->aux_jar = temp;
    }

    s->jar->pos = 0;
}

static void walk_screens(bonk_select_t *s)
{
    const struct xcb_setup_t *setup = xcb_get_setup(s->conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    for (;iter.rem;xcb_screen_next(&iter)) {
        xcb_screen_t *screen = iter.data;
        xcb_window_t root = screen->root;
        xcb_query_tree_cookie_t c = xcb_query_tree(s->conn, root);

        bonk_window_list_push(s->window_stack, root);
        bonk_cookie_push(s->jar, c);
        walk_for_windows(s);
    }
}

static void load_client_list(bonk_select_t *s)
{
    xcb_get_property_cookie_t c = xcb_ewmh_get_client_list(s->ewmh, 0);
    xcb_ewmh_get_windows_reply_t r;

    /* Assume that various screens all have the same client list, since they'd
       all be managed by the same window manager. Adjust this to walk screens if
       that turns out to not be true. */
    if (xcb_ewmh_get_client_list_reply(s->ewmh, c, &r, NULL) == 0)
        return;

    xcb_window_t *window_list = r.windows;
    uint32_t i;

    for (i = 0;i < r.windows_len;i++)
        bonk_window_list_push(s->window_stack, window_list[i]);

    xcb_ewmh_get_windows_reply_wipe(&r);
}

int bonk_select_exec(bonk_state_t *b, bonk_select_t *s)
{
    if ((s->mask & B_USE_CLIENT_LIST) == 0)
        walk_screens(s);
    else
        load_client_list(s);

    if (s->mask & B_ONLY_VISIBLE)
        filter_visible(s);

    if (s->mask & (B_SELECT_CLASS | B_SELECT_INSTANCE))
        filter_class_and_instance(s);

    if (s->mask & B_HAS_PROPERTY)
        filter_has_prop(s);

    if (s->mask & B_SELECT_TITLE)
        filter_title(s);

    int result = s->window_stack->pos;

    /* Don't swap stacks until this is good and done. */
    if ((s->mask & B_RETRY) == 0 || s->window_stack->pos) {
        bonk_window_list *temp = b->window_stack;

        b->window_stack = s->window_stack;
        s->window_stack = temp;
        result = b->window_stack->pos;
    }

    return result;
}
