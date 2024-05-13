#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    set-window --class xterm- \
                    set-window --wait --class xterm \
                    set-window --instance xterm- \
                    set-window --wait --instance xterm \
                    set-window --title xterm- \
                    set-window --window $WID --wait --title xterm \
                    set-window --desktop 0 \
                    set-window --desktop 1 \
                    set-window --desktop -1 \
                    set-window --delete WM_ICON_NAME \
                    set-window --machine hello

exit $?
