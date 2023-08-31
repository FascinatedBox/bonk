#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

#include "bonk_internal.h"
#include "bonk_select.h"

extern void bonk_arg_require_n(bonk_state_t *, int);

#define CRITERIA_CASE(criteria, flag) \
case opt_match##criteria: \
    ret = bonk_select_set_criteria(s, flag, 1, optarg); \
    break; \
case opt_exact##criteria: \
    ret = bonk_select_set_criteria(s, flag, 0, optarg); \
    break;

typedef enum {
    opt_exact_class,
    opt_exact_classname,
    opt_exact_instance,
    opt_exact_title,
    opt_has_property,
    opt_has_state,
    opt_match_class,
    opt_match_classname,
    opt_match_instance,
    opt_match_title,
    opt_show,
    opt_help = 'h',
} optlist_t;

struct option longopts[] = {
    { "classname", required_argument, NULL, opt_match_classname },
    { "class", required_argument, NULL, opt_match_class },
    { "exact-classname", required_argument, NULL, opt_exact_classname },
    { "exact-class", required_argument, NULL, opt_exact_class },
    { "exact-instance", required_argument, NULL, opt_exact_instance },
    { "exact-title", required_argument, NULL, opt_exact_title },
    { "has-property", required_argument, NULL, opt_has_property },
    { "has-state", required_argument, NULL, opt_has_state },
    { "instance", required_argument, NULL, opt_match_instance },
    { "show", no_argument, NULL, opt_show },
    { "title", required_argument, NULL, opt_match_title },
    { "help", no_argument, NULL, opt_help },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options]\n"
    "--<criteria> <pattern>       match against regexp <pattern>\n"
    "--exact-<criteria> <string>  must be exactly <string>\n"
    "--has-property <property>    window has <property> set to any value\n"
    "--has-state <atom>           window has <atom> in _NET_WM_STATE\n"
    "                             (see 'bonk state --help' for a list).\n"
    "\n"
    "--retry                      retry until a result is found\n"
    "--sync                       wait until a result is found\n"
    "\n"
    "-h, --help                   display this help and exit\n"
    "\n"
    "Criteria can be any of the following:\n"
    "  class, classname, instance, title\n"
    ;

int b_reject(bonk_state_t *b)
{
    bonk_select_t *s = bonk_new_select(b);
    int ret = 1;
    int show = 0;
    int retry = 0;

    bonk_select_set_is_reject(s);

    BONK_GETOPT_LOOP(c, b, "+h", longopts) {
        switch (c) {
            CRITERIA_CASE(_class, B_SELECT_CLASS)
            CRITERIA_CASE(_instance, B_SELECT_INSTANCE)
            CRITERIA_CASE(_title, B_SELECT_TITLE)
            case opt_match_classname:
                fprintf(stderr, "bonk reject warning: use instance instead of classname (ok for now).\n");
                ret = bonk_select_set_criteria(s, B_SELECT_INSTANCE, 1, optarg);
                break;
            case opt_exact_classname:
                fprintf(stderr, "bonk reject warning: use instance instead of classname (ok for now).\n");
                ret = bonk_select_set_criteria(s, B_SELECT_INSTANCE, 0, optarg);
                break;
            case opt_has_property:
                bonk_select_set_has_property(s, optarg);
                break;
            case opt_has_state:
                ret = bonk_select_set_has_state(s, b, optarg);
                break;
            case opt_show:
                show = 1;
                break;
            BONK_GETOPT_HELP
        }

        if (ret == 0) {
            const char *error = bonk_select_error_str(s);

            fprintf(stderr, "bonk reject error: '%s' %s\n", optarg, error);
            return 0;
        }
    }

    bonk_arg_require_n(b, 0);
    bonk_select_exec(b, s);

    if (b->argc == 0 || show == 1) {
        int i;
        for (i = 0;i < b->window_stack->pos;i++) {
            printf("0x%.8x\n", b->window_stack->data[i]);
        }
    }

    bonk_free_select(s);
    return 1;
}
