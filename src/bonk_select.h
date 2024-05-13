#ifndef BONK_SELECT_H
# define BONK_SELECT_H
# include <regex.h>

# include "bonk_internal.h"

# define B_SELECT_CLASS       (1 << 0)
# define B_SELECT_DESKTOP_ID  (1 << 1)
# define B_SELECT_INSTANCE    (1 << 2)
# define B_SELECT_PID         (1 << 3)
# define B_SELECT_TITLE       (1 << 4)

# define B_HAS_PROPERTY       (1 << 5)
# define B_HAS_STATE          (1 << 6)
# define B_IS_REJECT          (1 << 7)
# define B_ONLY_VISIBLE       (1 << 8)
# define B_RETRY              (1 << 9)
# define B_USE_CLIENT_LIST    (1 << 10)

typedef enum {
    ec_ok,
    ec_bad_desktop_id,
    ec_bad_pattern,
    ec_bad_pid,
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
    int32_t desktop_id;
    int32_t pid;

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
void bonk_select_set_desktop_id(bonk_select_t *, int32_t);
int bonk_select_set_desktop_id_str(bonk_select_t *, const char *);
void bonk_select_set_show_hidden(bonk_select_t *);
void bonk_select_use_client_list(bonk_select_t *);
void bonk_select_set_retry(bonk_select_t *);
void bonk_select_set_has_property(bonk_select_t *, const char *);
int bonk_select_set_has_state(bonk_select_t *, bonk_state_t *, const char *);
void bonk_select_set_is_reject(bonk_select_t *);
void bonk_select_set_pid(bonk_select_t *, int32_t);
int bonk_select_set_pid_str(bonk_select_t *, const char *);
int bonk_select_set_criteria(bonk_select_t *s,
                             unsigned int criteria,
                             int is_pattern,
                             const char *text);

#endif
