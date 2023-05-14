#!/usr/bin/env bash
# run-with-env.sh: Execute <commands> within Xephyr, closing Xephyr on exit.
#
# You may need to run 'xauth add :1 . `mcookie`' to allow Xephyr to work.

./bonk select -c --all --class Xephyr close 2> /dev/null
unset XDG_SEAT
Xephyr -br -ac -noreset -screen 1280x720 :1 &
./bonk select -c --retry --instance xephyr > /dev/null
INITIAL_DISPLAY=$DISPLAY
DISPLAY=":1"
xterm 2> /dev/null &
# Don't use -c here because there's no wm to provide a client list.
./bonk select --retry --instance xterm > /dev/null
"$@"
pkill Xephyr
