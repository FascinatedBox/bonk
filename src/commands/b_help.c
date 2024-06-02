#include <stdio.h>

#include "bonk_internal.h"

int b_help(bonk_state_t *b)
{
    if (b->argc > 1)
        bonk_exec_man_for(b->argv[1]);

    puts(
    "Usage: bonk <commands [arguments]>\n"
    "\n"
    "window stack management\n"
    "    get-active    Use window manager's active window\n"
    "    get-focus     Use the window that currently has input focus\n"
    "    pick          Left click to select a window\n"
    "    reject        Remove windows using criteria\n"
    "    select        Search for a window using criteria\n"
    "\n"
    "window actions\n"
    "    activate      Tell window manager to set active window\n"
    "    close         Attempt to close a window\n"
    "    decoration    Set MOTIF (titlebar + border) hints\n"
    "    focus         Give input focus to a window\n"
    "    lower         Push a window to the back, behind other windows\n"
    "    map           Reveal a window a window hidden with unmap\n"
    "    move-resize   Specify an exact size and place for a window\n"
    "    opacity       Adjust window transparency\n"
    "    raise         Pull a window to the front, ahead of other windows\n"
    "    set-window    Update simple properties (class, instance, etc.)\n"
    "    state         Minimize, maximize, fullscreen, and more\n"
    "    terminate     Terminate program associated with a window\n"
    "    unmap         Hide a window (does not minimize to taskbar)\n"
    "\n"
    "information\n"
    "    get-title     Get title of a window\n"
    "\n"
    "miscellaneous\n"
    "    sleep         Suspend execution for a given duration\n"
    );

    exit(EXIT_SUCCESS);
    return 0;
}
