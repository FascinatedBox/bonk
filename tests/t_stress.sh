#!/usr/bin/env bash

xeyes 2> /dev/null &
XEYES_WID=`./bonk select --retry --instance xeyes`
./bonk prop-delete -w $XEYES_WID WM_CLASS
./bonk prop-adjust -w $XEYES_WID --class xeyes
./bonk prop-delete -w $XEYES_WID WM_CLASS
./bonk prop-adjust -w $XEYES_WID --instance xeyes
./bonk prop-delete -w $XEYES_WID sphinxofblackquartzjudgemyvow
./bonk state -w $XEYES_WID
./bonk state -w $XEYES_WID -a sphinxofblackquartzjudgemyvow
./bonk state -w $XEYES_WID -t above -t above -t above -t above -t above \
						   -t above -t above -t above -t above -t above \
						   -t above -t above -t above -t above -t above
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
exit 0
