#include <string.h>

#include "bonk_select.h"

bonk_select_t *bonk_new_select(bonk_state_t *b)
{
    bonk_select_t *s = malloc(sizeof(*s));

    s->conn = b->conn;
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
