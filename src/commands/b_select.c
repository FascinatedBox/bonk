#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

#include "bonk_internal.h"
#include "bonk_select.h"

#define CRITERIA_CASE(criteria, flag) \
case opt_match##criteria: \
    ret = bonk_select_set_criteria(s, flag, 1, optarg); \
    break; \
case opt_exact##criteria: \
    ret = bonk_select_set_criteria(s, flag, 0, optarg); \
    break;

typedef enum {
    opt_all,
    opt_desktop,
    opt_exact_class,
    opt_exact_instance,
    opt_exact_title,
    opt_has_property,
    opt_has_state,
    opt_help,
    opt_if_empty_stack,
    opt_match_class,
    opt_match_instance,
    opt_match_title,
    opt_pid,
    opt_retry,
    opt_show,
    opt_clients = 'c',
} optlist_t;

static struct option longopts[] = {
    { "all", no_argument, NULL, opt_all },
    { "class", required_argument, NULL, opt_match_class },
    { "clients", no_argument, NULL, opt_clients },
    { "desktop", required_argument, NULL, opt_desktop },
    { "exact-class", required_argument, NULL, opt_exact_class },
    { "exact-instance", required_argument, NULL, opt_exact_instance },
    { "exact-title", required_argument, NULL, opt_exact_title },
    { "has-property", required_argument, NULL, opt_has_property },
    { "has-state", required_argument, NULL, opt_has_state },
    { "if-empty-stack", no_argument, NULL, opt_if_empty_stack },
    { "instance", required_argument, NULL, opt_match_instance },
    { "pid", required_argument, NULL, opt_pid },
    { "retry", no_argument, NULL, opt_retry },
    { "show", no_argument, NULL, opt_show },
    { "title", required_argument, NULL, opt_match_title },
    { "help", no_argument, NULL, opt_help },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options]\n"
    "\n"
    "Load windows into the window stack\n"
    "\n"
    "--<criteria> <pattern>       match against regexp <pattern>\n"
    "--exact-<criteria> <string>  must be exactly <string>\n"
    "--has-property <property>    window has <property> set to any value\n"
    "--has-state <atom>           window has <atom> in _NET_WM_STATE\n"
    "                             (see 'bonk state --help' for a list).\n"
    "--desktop <n>                only windows on desktop <n> (-1 for all)\n"
    "--pid <n>                    windows created by <pid>\n"
    "\n"
    "--if-empty-stack             only search if the window stack is empty\n"
    "--retry                      retry until a result is found\n"
    "\n"
    "--all                        include hidden windows\n"
    "-c, --clients                use managed clients\n"
    "                             (wm must support _NET_CLIENT_LIST)\n"
    "--show                       print the window stack\n"
    "\n"
    "Criteria can be any of the following:\n"
    "  class, instance, title\n"
    ;

int b_select(bonk_state_t *b)
{
    bonk_select_t *s = bonk_new_select(b);
    int if_empty_stack = 0;
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
            case opt_desktop:
                ret = bonk_select_set_desktop_id_str(s, optarg);
                break;
            case opt_has_property:
                bonk_select_set_has_property(s, optarg);
                break;
            case opt_has_state:
                ret = bonk_select_set_has_state(s, b, optarg);
                break;
            case opt_if_empty_stack:
                if_empty_stack = 1;
                break;
            case opt_pid:
                ret = bonk_select_set_pid_str(s, optarg);
                break;
            case opt_retry:
                bonk_select_set_retry(s);
                retry = 1;
                break;
            case opt_show:
                show = 1;
                break;
            BONK_GETOPT_HELP
        }

        if (ret == 0) {
            const char *error = bonk_select_error_str(s);

            fprintf(stderr, "bonk select error: '%s' %s\n", optarg, error);
            return 0;
        }
    }

    bonk_arg_require_n(b, 0);

    if ((if_empty_stack && b->window_stack->pos) == 0) {
        while (1) {
            int result = bonk_select_exec(b, s);

            if (retry == 0 || result)
                break;

            usleep(500000);
        }
    }

    if (b->argc == 0 || show == 1) {
        int i;
        for (i = 0;i < b->window_stack->pos;i++) {
            BONK_PRINT_WINDOW(b->window_stack->data[i]);
        }
    }

    bonk_free_select(s);
    return 1;
}
