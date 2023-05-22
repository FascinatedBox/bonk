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
    opt_all,
    opt_clients = 'c',
    opt_exact_class,
    opt_exact_classname,
    opt_exact_instance,
    opt_exact_title,
    opt_has_property,
    opt_has_state,
    opt_help,
    opt_match_class,
    opt_match_classname,
    opt_match_instance,
    opt_match_title,
    opt_retry,
    opt_show,
    opt_sync,
} optlist_t;

struct option longopts[] = {
    { "all", no_argument, NULL, opt_all },
    { "classname", required_argument, NULL, opt_match_classname },
    { "class", required_argument, NULL, opt_match_class },
    { "clients", no_argument, NULL, opt_clients },
    { "exact-classname", required_argument, NULL, opt_exact_classname },
    { "exact-class", required_argument, NULL, opt_exact_class },
    { "exact-instance", required_argument, NULL, opt_exact_instance },
    { "exact-title", required_argument, NULL, opt_exact_title },
    { "has-property", required_argument, NULL, opt_has_property },
    { "has-state", required_argument, NULL, opt_has_state },
    { "instance", required_argument, NULL, opt_match_instance },
    { "retry", no_argument, NULL, opt_retry },
    { "show", no_argument, NULL, opt_show },
    { "sync", no_argument, NULL, opt_sync },
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
    "--all                        include hidden windows\n"
    "-c, --clients                use managed clients\n"
    "                             (wm must support _NET_CLIENT_LIST)\n"
    "-h, --help                   display this help and exit\n"
    "\n"
    "Criteria can be any of the following:\n"
    "  class, classname, instance, title\n"
    ;

int b_select(bonk_state_t *b)
{
    bonk_select_t *s = bonk_new_select(b);
    int ret = 1;
    int show = 0;
    int retry = 0;

    BONK_GETOPT_LOOP(c, b, "+ch", longopts) {
        switch (c) {
            CRITERIA_CASE(_class, B_SELECT_CLASS)
            CRITERIA_CASE(_instance, B_SELECT_INSTANCE)
            CRITERIA_CASE(_title, B_SELECT_TITLE)
            case opt_clients:
                bonk_select_use_client_list(s);
                break;
            case opt_all:
                bonk_select_set_show_hidden(s);
                break;
            case opt_match_classname:
                fprintf(stderr, "bonk select warning: use instance instead of classname (ok for now).\n");
                ret = bonk_select_set_criteria(s, B_SELECT_INSTANCE, 1, optarg);
                break;
            case opt_exact_classname:
                fprintf(stderr, "bonk select warning: use instance instead of classname (ok for now).\n");
                ret = bonk_select_set_criteria(s, B_SELECT_INSTANCE, 0, optarg);
                break;
            case opt_has_property:
                bonk_select_set_has_property(s, optarg);
                break;
            case opt_has_state:
                ret = bonk_select_set_has_state(s, b, optarg);
                break;
            case opt_sync:
                fprintf(stderr, "bonk select warning: use retry instead of sync (ok for now).\n");
            case opt_retry:
                bonk_select_set_retry(s);
                retry = 1;
                break;
            case opt_show:
                show = 1;
                break;
            case opt_help:
                bonk_usage(b, usage);
                break;
            default:
                return 0;
        }

        if (ret == 0) {
            const char *error = bonk_select_error_str(s);

            fprintf(stderr, "bonk select error: '%s' %s\n", optarg, error);
            return 0;
        }
    }

    bonk_arg_require_n(b, 0);

    while (1) {
        int result = bonk_select_exec(b, s);

        if (retry == 0 || result)
            break;

        usleep(500000);
    }

    if (b->argc == 0 || show == 1) {
        int i;
        for (i = 0;i < b->window_stack->pos;i++) {
            fprintf(stdout, "0x%.8x\n", b->window_stack->data[i]);
        }
    }

    bonk_free_select(s);
    return 1;
}
