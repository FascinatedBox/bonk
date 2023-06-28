#ifndef BONK_SELECT_H
# define BONK_SELECT_H
# include <regex.h>

# include "bonk_internal.h"

# define B_SELECT_CLASS       0x001
# define B_SELECT_INSTANCE    0x002
# define B_SELECT_TITLE       0x004

# define B_HAS_PROPERTY       0x008
# define B_HAS_STATE          0x010
# define B_IS_REJECT          0x020
# define B_ONLY_VISIBLE       0x040
# define B_RETRY              0x080
# define B_USE_CLIENT_LIST    0x100

typedef enum {
    ec_ok,
    ec_bad_pattern,
    ec_bad_state_name,
} bonk_select_error_code;

typedef union {
    char *text;
    regex_t pattern;
} bonk_criteria;

typedef struct {
    xcb_connection_t *conn;

    bonk_cookie_jar *jar;
    bonk_cookie_jar *aux_jar;
    bonk_window_list *window_stack;
    xcb_ewmh_connection_t *ewmh;

    bonk_criteria class_criteria;
    bonk_criteria instance_criteria;
    bonk_criteria title_criteria;
    xcb_atom_t has_prop;
    xcb_atom_t has_state_atom;

    bonk_select_error_code error_code;
    unsigned int mask;
    unsigned int rxmask;
} bonk_select_t;

bonk_cookie_jar *bonk_new_cookie_jar(int);
void bonk_free_cookie_jar(bonk_cookie_jar *);
void bonk_cookie_jar_push(bonk_cookie_jar *, xcb_void_cookie_t);

#define bonk_cookie_push(j_, c_) \
    bonk_cookie_jar_push(j_, *(xcb_void_cookie_t *)&c_)

#define BONK_INIT_COOKIE_LIST(type_name, var_name) \
    type_name var_name = (type_name)(s->jar->data)

#define bonk_cookie_clear(jar) \
    jar->pos = 0

bonk_select_t *bonk_new_select(bonk_state_t *);
void bonk_free_select(bonk_select_t *);
int bonk_select_exec(bonk_state_t *, bonk_select_t *);

const char *bonk_select_error_str(bonk_select_t *);
void bonk_select_set_show_hidden(bonk_select_t *);
void bonk_select_use_client_list(bonk_select_t *);
void bonk_select_set_retry(bonk_select_t *);
void bonk_select_set_has_property(bonk_select_t *, const char *);
int bonk_select_set_has_state(bonk_select_t *, bonk_state_t *, const char *);
void bonk_select_set_is_reject(bonk_select_t *);
int bonk_select_set_criteria(bonk_select_t *s,
                             unsigned int criteria,
                             int is_pattern,
                             const char *text);

#endif
