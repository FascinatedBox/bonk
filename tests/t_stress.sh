#!/usr/bin/env bash

xeyes 2> /dev/null &
XEYES_WID=`./bonk select --retry --instance xeyes`
./bonk set-window -w $XEYES_WID --delete WM_CLASS
./bonk set-window -w $XEYES_WID --class xeyes
./bonk set-window -w $XEYES_WID --delete WM_CLASS
./bonk set-window -w $XEYES_WID --instance xeyes
./bonk set-window -w $XEYES_WID --delete sphinxofblackquartzjudgemyvow
./bonk state -w $XEYES_WID
./bonk state -w $XEYES_WID -a sphinxofblackquartzjudgemyvow
./bonk state -w $XEYES_WID -t above -t above -t above -t above -t above \
						   -t above -t above -t above -t above -t above \
						   -t above -t above -t above -t above -t above
./bonk set-window -w $XEYES_WID --delete WM_STATE --delete WM_STATE \
                                --delete WM_STATE --delete WM_STATE \
                                --delete WM_STATE --delete WM_STATE \
                                --delete WM_STATE --delete WM_STATE \
                                --delete WM_STATE --delete WM_STATE
./bonk get-focus state || exit 99
pkill xeyes
./bonk quickbrownfox && exit 99
SAVE_DISPLAY=$DISPLAY
DISPLAY=:25
./bonk get-focus && exit 99
DISPLAY=$SAVE_DISPLAY
./bonk get-focus raise %-1 || exit 99 # Valid index, the rest are invalid.
./bonk get-focus raise %-2 && exit 99
./bonk get-focus raise %2 && exit 99
./bonk get-focus raise %J && exit 99
./bonk get-focus raise %0A && exit 99
./bonk get-focus raise %- && exit 99
./bonk get-focus raise % && exit 99
./bonk select -c
./bonk select --all --instance xterm unmap select --instance xterm raise && exit 99
./bonk select --all --instance xterm map
./bonk select --exact-instance ""
./bonk select --class "*" && exit 99
./bonk select --desktop -2 && exit 99
./bonk select --desktop 42069 && exit 99
./bonk select --desktop 1a && exit 99
./bonk select --desktop lol && exit 99
./bonk select --limit -2 && exit 99
./bonk select --limit 0 && exit 99
./bonk select --limit 42069 && exit 99
./bonk select --limit 1a && exit 99
./bonk select --limit lol && exit 99
./bonk select --pid -1 && exit 99
./bonk select --pid 123456789012 && exit 99
./bonk select --pid 1a && exit 99
./bonk select --pid lol && exit 99
./bonk get-focus set-window --desktop 1a && exit 99
./bonk get-focus set-window --desktop aa && exit 99
./bonk get-focus set-window --desktop -2 && exit 99
./bonk sleep 1J && exit 99
./bonk sleep -1s && exit 99
exit 0
