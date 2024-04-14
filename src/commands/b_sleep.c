/*
    I copied GNU Bash's `sleep` to implement Bonk's sleep. Both are GPLv2. The
    following is the license text copied from Bash:

    Copyright (C) 1999-2021 Free Software Foundation, Inc.

    This file is part of GNU Bash.
    Bash is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bash is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "bonk_internal.h"

extern void bonk_arg_require_n(bonk_state_t *, int);

typedef enum {
    opt_help = 'h',
} optlist_t;

static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { NULL, 0, NULL, 0 },
};

static const char *usage =
    "Usage: %s [options] number[.suffix]\n"
    "\n"
    "Temporarily suspend execution.\n"
    "\n"
    "By default, pause for <number> seconds. Fractional amounts are allowed.\n"
    "Additionally, the following suffixes are supported:\n"
    "\n"
    "s - seconds\n"
    "m - minutes\n"
    "h - hours\n"
    "d - days\n"
    "\n"
    "Suffixes and fractional amounts can be combined (ex: 1m20s, .2d)\n"
    "\n"
    "-h, --help               display this help and exit\n"
    ;

#define DECIMAL   '.'
#define DIGIT(c)  ((c) >= '0' && (c) <= '9')
#define RETURN(x) \
do { \
    if (ip) *ip = ipart * mult; \
    if (up) *up = upart; \
    if (ep) *ep = p; \
    return (x); \
} while (0)
#define S_SEC   1
#define S_MIN   (60*S_SEC)
#define S_HOUR  (60*S_MIN)
#define S_DAY   (24*S_HOUR)

static int multiplier[7] = { 1, 100000, 10000, 1000, 100, 10, 1 };

int uconvert(char *s, long *ip, long *up, char **ep)
{
    int n, mult;
    long ipart, upart;
    char *p;

    ipart = upart = 0;
    mult = 1;

    if (s && (*s == '-' || *s == '+')) {
        mult = (*s == '-') ? -1 : 1;
        p = s + 1;
    }
    else
        p = s;

    for ( ; p && *p; p++) {
        if (*p == DECIMAL)        /* decimal point */
            break;
        if (DIGIT(*p) == 0)
            RETURN(0);
        ipart = (ipart * 10) + (*p - '0');
    }

    if (p == 0 || *p == 0) /* Silence warnings. */
        RETURN(1);

    if (*p == DECIMAL)
        p++;

    /* Look for up to six digits past a decimal point. */
    for (n = 0; n < 6 && p[n]; n++) {
        if (DIGIT(p[n]) == 0) {
            if (ep) {
                upart *= multiplier[n];
                p += n;       /* To set EP */
            }
            RETURN(0);
        }
        upart = (upart * 10) + (p[n] - '0');
    }

    /* Now convert to millionths */
    upart *= multiplier[n];

    if (n == 6 && p[6] >= '5' && p[6] <= '9')
        upart++; /* round up 1 */

    if (ep) {
        p += n;
        while (DIGIT(*p))
            p++;
    }

    RETURN(1);
}

static int parse_gnutimefmt (char *string, long *sp, long *up)
{
    int c, r;
    char *s, *ep;
    long tsec, tusec, accumsec, accumusec, t;
    int mult;

    tsec = tusec = 0;
    accumsec = accumusec = 0;
    mult = 1;

    for (s = string; s && *s; s++) {
        r = uconvert(s, &accumsec, &accumusec, &ep);
        if (r == 0 && *ep == 0)
            return r;
        c = *ep;
        mult = 1;
        switch (c) {
            case '\0':
            case 's':
                mult = S_SEC;
                break;
            case 'm':
                mult = S_MIN;
                break;
            case 'h':
                mult = S_HOUR;
                break;
            case 'd':
                mult = S_DAY;
                break;
            default:
                return 0;
        }

        /* multiply the accumulated value by the multiplier */
        t = accumusec * mult;
        accumsec = accumsec * mult + (t / 1000000);
        accumusec = t % 1000000;

        /* add to running total */
        tsec += accumsec;
        tusec += accumusec;
        if (tusec >= 1000000) {
            tsec++;
            tusec -= 1000000;
        }

        /* reset and continue */
        accumsec = accumusec = 0;
        mult = 1;
        if (c == 0)
            break;
        s = ep;
    }

    if (sp)
        *sp = tsec;
    if (up)
        *up = tusec;

    return 1;
}

int fsleep(unsigned int sec, unsigned int usec)
{
    int e, r;
    sigset_t blocked, prevmask;
    struct timespec ts;
    sigemptyset (&blocked);

    ts.tv_sec = sec;
    ts.tv_nsec = usec * 1000;

    do {
        r = pselect(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &ts, &blocked);
        e = errno;
        if (r < 0 && errno == EINTR)
            return -1; /* caller will handle */
        errno = e;
    } while (r < 0 && errno == EINTR);

    return r;
}

int b_sleep(bonk_state_t *b)
{
    BONK_GETOPT_LOOP(c, b, "+h", longopts) {
        switch (c) {
            BONK_GETOPT_HELP
        }
    }

    bonk_arg_require_n(b, 1);
    char *s = bonk_arg_next_unchecked(b);
    char *ep;
    long sec, usec;

    int result = uconvert(s, &sec, &usec, &ep);
    /*
     * Maybe postprocess conversion failures here based on EP
     *
     * A heuristic: if the conversion failed, but the argument appears to
     * contain a GNU-like interval specifier (e.g. "1m30s"), try to parse
     * it. If we can't, return the right exit code to tell
     * execute_builtin to try and execute a disk command instead.
     */
    if (result == 0 && (strchr("dhms", *ep) || strpbrk(s, "dhms")))
        result = parse_gnutimefmt(s, &sec, &usec);

    if (result == 0)
        fprintf(stderr, "bonk sleep error: Bad sleep interval '%s'.\n", s);
    else
        fsleep(sec, usec);

    return result;
}
