bonk - X11 toolkit
==================

### Overview

bonk - **B**ox's wind**O**w ma**N**ager tool**K**it. Bonk is a command-line
tool that provides subcommands to modify windows, adjust window properties,
change window decorations, and more.

Bonk is designed with X11 in mind.

### Requirements

A C compiler, CMake, and libxcb to build Bonk. Running the testing suite
requires Python 3, Xephyr, xtrace, and lcov. The following will build Bonk:

```
cmake .
make
sudo make install
```

### Examples

Bonk executes commands based on what is read from the command line. Here's a
starter example:

```
bonk get-focus lower
```

The first command, `get-focus`, places the current window into the window stack.
The second command, `lower`, moves a window to the very back of the desktop.
Commands that operate on a window **always default to the first window in the
stack**.

How about making this more interactive?

```
bonk pick lower get-title
```

The `pick` command prompts the user to select a window by left-clicking on it.
Selection can be cancelled by using any other mouse press. Like `get-focus`,
`pick` loads a window into the window stack for `lower` and `get-title` to
operate on.

What about using a specific window instead?

```
bonk raise --window 0xdeadbeef get-title
```

In this case, a window id is specified by the `--window` argument to `raise`.
The `--window` argument is accepted by all commands that operate on windows. It
is important to note that specifying a window **replaces all other windows in
the window stack**.

What about finding windows that match a certain criteria?

```
bonk select --class firefox
```

This will return all visible windows that were created by firefox. To return
minimized windows, add `--all` to the criteria.

It's possible to narrow the search further:

```
bonk select --all --class firefox --title Example
```

This returns windows created by firefox that have 'Example' in their title. The
`--all` option means to include windows that are currently minimized.

The above search may return windows that are of no use to the user. How about
limiting the search to windows that are only in the taskbar?

```
bonk select --clients
```

Most window managers provide a list of windows that they're managing. Adding
`--clients` to search criteria provides a way to only search within that list,
to prevent turning up windows that are of no interest to the user.

### Window stack

The `window-arg` property on a command allows the following:

* `%N`: N-th window (first window is `%0`)
* `%-N`: Select from the end of the stack (last window is `%-1`)
* `%@`: Select all windows.

```
%-1: Last argument
%@: All windows
```

### Testing

Execute `./run-tests.sh` to run all tests.

### Command reference

Here are all commands that Bonk supports (output of `bonk help`).

```
Usage: bonk <commands [arguments]>

window stack management
    get-active    Use window manager's active window
    get-focus     Use the window that currently has input focus
    pick          Left click to select a window
    reject        Remove windows using criteria
    select        Search for a window using criteria

window actions
    activate      Tell window manager to set active window
    close         Attempt to close a window
    decoration    Set MOTIF (titlebar + border) hints
    focus         Give input focus to a window
    lower         Push a window to the back, behind other windows
    map           Reveal a window a window hidden with unmap
    move-resize   Specify an exact size and place for a window
    raise         Pull a window to the front, ahead of other windows
    state         Minimize, maximize, fullscreen, and more
    unmap         Hide a window (does not minimize to taskbar)

information
    get-title     Get title of a window

properties
    prop-adjust   Update a simple property (class, instance, etc.)
    prop-delete   Remove a property from a window (use with caution)
```
