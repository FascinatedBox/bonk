#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
./bonk opacity -w $WID --min -1.0
./bonk opacity -w $WID --min 1.01
./bonk opacity -w $WID --min 1e
./bonk opacity -w $WID --max -1.0
./bonk opacity -w $WID --max 1e
./bonk opacity -w $WID --max 1.01
./bonk opacity -w $WID --inc -1.0
./bonk opacity -w $WID --inc 1e
./bonk opacity -w $WID --inc 1.01
./bonk opacity -w $WID --dec -1.0
./bonk opacity -w $WID --dec 1e
./bonk opacity -w $WID --dec 1.01
./bonk opacity -w $WID --inc 0.5 --dec 0.5
./bonk opacity -w $WID --min 0.75 --max 0.25
./bonk set-window -w $WID --delete _NET_WM_WINDOW_OPACITY

./bonk opacity -w $WID --dec .25 \
                    opacity -w $WID --dec .25 \
                    opacity -w $WID --dec .25 \
                    opacity -w $WID --dec .25 \
                    opacity -w $WID --inc .25 \
                    opacity -w $WID --inc .25 \
                    opacity -w $WID --inc .25 \
                    opacity -w $WID --inc .25 \
                    opacity --wait -w $WID --dec 1.0 \
                    opacity -w $WID --inc 1 \
                    opacity -w $WID --min .5 --dec 1 \
                    get-focus \
                    opacity -w $WID --max .75 --inc 1 \
                    opacity -w $WID --min 0.25 --max 0.25

exit $?
