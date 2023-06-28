#include <string.h>

#include "bonk_select.h"

bonk_select_t *bonk_new_select(bonk_state_t *b)
{
    bonk_select_t *s = malloc(sizeof(*s));

    s->conn = b->conn;
    s->error_code = ec_ok;
    s->mask = B_ONLY_VISIBLE;
    s->rxmask = 0;
    s->jar = bonk_new_cookie_jar(64);
    s->aux_jar = bonk_new_cookie_jar(64);
    s->window_stack = bonk_new_window_list(64);
    s->ewmh = b->ewmh;

    return s;
}

#define FREE_CRITERIA(flag, criteria) \
{ \
  if (s->rxmask & flag) \
    regfree(&s->criteria.pattern); \
  else if (s->mask & flag) \
    free(s->criteria.text); \
}

void bonk_free_select(bonk_select_t *s)
{
    FREE_CRITERIA(B_SELECT_CLASS,    class_criteria)
    FREE_CRITERIA(B_SELECT_INSTANCE, instance_criteria)
    FREE_CRITERIA(B_SELECT_TITLE,    title_criteria)
    bonk_free_cookie_jar(s->jar);
    bonk_free_cookie_jar(s->aux_jar);
    bonk_free_window_list(s->window_stack);
    free(s);
}

#define SET_CRITERIA(criteria) \
{ \
  if (is_pattern == 0) \
    s->criteria.text = strdup(text); \
  else \
    result = !regcomp(&s->criteria.pattern, text, REG_EXTENDED | REG_ICASE); \
  \
}

int bonk_select_set_criteria(bonk_select_t *s,
                             unsigned int criteria,
                             int is_pattern,
                             const char *text)
{
    int result = 1;

    if (criteria == B_SELECT_CLASS)
        SET_CRITERIA(class_criteria)
    else if (criteria == B_SELECT_INSTANCE)
        SET_CRITERIA(instance_criteria)
    else if (criteria == B_SELECT_TITLE)
        SET_CRITERIA(title_criteria)

    if (result) {
        if (is_pattern)
            s->rxmask |= criteria;

        s->mask |= criteria;
    }
    else
        s->error_code = ec_bad_pattern;

    return (s->error_code == ec_ok);
}

const char *bonk_select_error_str(bonk_select_t *s)
{
    const char *result = NULL;

    switch (s->error_code) {
        case ec_bad_pattern:
            result = "is not a valid match pattern.";
            break;
        case ec_bad_state_name:
            result = "is not a valid atom state name.";
            break;
        case ec_ok:
            result = "";
            break;
    }

    return result;
}

void bonk_select_set_show_hidden(bonk_select_t *s)
{
    s->mask &= ~B_ONLY_VISIBLE;
}

void bonk_select_use_client_list(bonk_select_t *s)
{
    s->mask |= B_USE_CLIENT_LIST;
}

void bonk_select_set_retry(bonk_select_t *s)
{
    s->mask |= B_RETRY;
}

void bonk_select_set_has_property(bonk_select_t *s, const char *prop_name)
{
    s->mask |= B_HAS_PROPERTY;
    s->has_prop = bonk_atom_find_or_intern(s->conn, prop_name);
}

int bonk_select_set_has_state(bonk_select_t *s, bonk_state_t *b,
        const char *prop_name)
{
    xcb_atom_t result = bonk_window_state_atom_from_string(b, prop_name);

    if (result == XCB_ATOM_NONE)
        s->error_code = ec_bad_state_name;

    s->has_state_atom = result;
    s->mask |= B_HAS_STATE;
    return (result != XCB_ATOM_NONE);
}

void bonk_select_set_is_reject(bonk_select_t *s)
{
    s->mask |= B_IS_REJECT;
}
