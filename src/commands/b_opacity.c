#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "bonk_internal.h"

static xcb_atom_t net_opacity = XCB_ATOM_NONE;

#define FULL_OPACITY 0xffffffff
/* Incrementing 0.25 * 4 from 0.00 may not yield a perfect 1.00. Consider this
   to be close enough to perfect that it's fine to clear off the opacity. */
#define ITS_CLOSE_ENOUGH(o) (o >= 0xfffffff0)
#define INVALID_VALUE -1.00
#define OPT_INC 0x1
#define OPT_DEC 0x2

typedef enum {
    opt_dec,
    opt_help,
    opt_inc,
    opt_max,
    opt_min,
    opt_wait,
    opt_window = 'w',
} optlist_t;

static struct option longopts[] = {
    { "dec", required_argument, NULL, opt_dec },
    { "help", no_argument, NULL, opt_help },
    { "inc", required_argument, NULL, opt_inc },
    { "max", required_argument, NULL, opt_max },
    { "min", required_argument, NULL, opt_min },
    { "wait", no_argument, NULL, opt_wait },
    { "window", required_argument, NULL, opt_window },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options] [<window-arg>=%0]\n"
    "\n"
    "Adjust window transparency (note: 0.75 opacity = 25%% transparent)\n"
    "\n"
    "--dec <N>                Reduce opacity by <N%%>\n"
    "--inc <N>                Increase opacity by <N%%>\n"
    "\n"
    "--min <N>                Minimum possible opacity (default = 0.00)\n"
    "--max <N>                Maximum possible opacity (default = 1.00)\n"
    "\n"
    "--wait                   flush output buffer before next command\n"
    "-w, --window <wid>       add window <wid> to the stack\n"
    ;

static uint32_t get_opacity(bonk_state_t *b, xcb_window_t window)
{
    xcb_get_property_cookie_t c = xcb_get_property(
            b->conn,
            0, /* Do not delete this property. */
            window,
            net_opacity,
            XCB_ATOM_CARDINAL, 0, 4);

    uint32_t result;

    if (xcb_ewmh_get_cardinal_reply(b->ewmh, c, &result, NULL))
        return result;
    else
        return FULL_OPACITY;
}

static double scan_double(const char *str)
{
    char *str_end;

    double result = strtod(str, &str_end);

    if (result < 0.00 || result > 1.00 || *str_end != '\0')
        result = INVALID_VALUE;

    return result;
}

int b_opacity(bonk_state_t *b)
{
    double min_opacity = 0.00,
           opacity_max = 1.00,
           amount = 0.75;
    const char *min_arg = "0.00",
               *arg_max = "1.00";
    int inc_dec = 0, wait = 0;

    if (net_opacity == XCB_ATOM_NONE)
        net_opacity = bonk_atom_find_or_intern(b->conn,
            "_NET_WM_WINDOW_OPACITY");

    BONK_GETOPT_LOOP(c, b, "+hw:", longopts) {
        switch (c) {
            case opt_dec:
                amount = scan_double(optarg);
                if (amount == INVALID_VALUE) {
                    fprintf(stderr,
                        "bonk opacity error: Invalid dec amount '%s'.\n",
                        optarg);
                    return 0;
                }

                inc_dec |= OPT_DEC;
                break;
            case opt_inc:
                amount = scan_double(optarg);
                if (amount == INVALID_VALUE) {
                    fprintf(stderr,
                        "bonk opacity error: Invalid inc amount '%s'.\n",
                        optarg);
                    return 0;
                }

                inc_dec |= OPT_INC;
                break;
            case opt_min:
                min_opacity = scan_double(optarg);
                min_arg = optarg;

                if (min_opacity == INVALID_VALUE) {
                    fprintf(stderr,
                        "bonk opacity error: Invalid minimum '%s'.\n", optarg);
                    return 0;
                }

                break;
            case opt_max:
                opacity_max = scan_double(optarg);
                arg_max = optarg;

                if (opacity_max == INVALID_VALUE) {
                    fprintf(stderr,
                        "bonk opacity error: Invalid max '%s'.\n", optarg);
                    return 0;
                }

                break;
            BONK_GETOPT_COMMON
        }
    }

    bonk_arg_window_only(b);

    if (inc_dec == (OPT_DEC | OPT_INC)) {
        fprintf(stderr, "bonk opacity error: Cannot increase and decrease.\n");
        return 0;
    }

    if (min_opacity > opacity_max) {
        fprintf(stderr, "bonk opacity error: Minimum (%s) exceeds max (%s).\n",
                min_arg, arg_max);
        return 0;
    }

    BONK_FOREACH_WINDOW_DO(
        uint32_t current_opacity = get_opacity(b, iter_window);
        double iter_amount = (double) current_opacity / FULL_OPACITY;

        if (inc_dec & OPT_INC)
            iter_amount += amount;
        else if (inc_dec & OPT_DEC)
            iter_amount -= amount;

        if (iter_amount < min_opacity)
            iter_amount = min_opacity;

        if (iter_amount > opacity_max)
            iter_amount = opacity_max;

        iter_amount = iter_amount * FULL_OPACITY;

        uint32_t final = (uint32_t)iter_amount;

        if (ITS_CLOSE_ENOUGH(iter_amount))
            xcb_delete_property(b->conn, iter_window, net_opacity);
        else {
            xcb_change_property(b->conn,
                                XCB_PROP_MODE_REPLACE,
                                iter_window,
                                net_opacity,
                                XCB_ATOM_CARDINAL,
                                32,
                                1,
                                (unsigned char *)&final);
        }
    )

    if (wait)
        bonk_connection_flush(b);

    return 1;
}
