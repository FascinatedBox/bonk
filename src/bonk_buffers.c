#include "bonk_internal.h"

#define BONK_DECLARE_BUFFER(buffer_type, data_type) \
bonk_##buffer_type *bonk_new_##buffer_type(int initial) \
{ \
    bonk_##buffer_type *b = malloc(sizeof(*b)); \
 \
    b->data = malloc(initial * sizeof(*b->data)); \
    b->pos = 0; \
    b->size = initial; \
    return b; \
} \
void bonk_##buffer_type##_push(bonk_##buffer_type *b, data_type a) \
{ \
    if (b->pos + 1 > b->size) { \
        b->size *= 2; \
        b->data = realloc(b->data, b->size * sizeof(*b->data)); \
    } \
 \
    b->data[b->pos] = a; \
    b->pos++; \
} \
void bonk_free_##buffer_type(bonk_##buffer_type *b) \
{ \
    free(b->data); \
    free(b); \
}

BONK_DECLARE_BUFFER(cookie_jar, xcb_void_cookie_t)
BONK_DECLARE_BUFFER(window_list, xcb_window_t)
