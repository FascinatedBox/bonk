#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk move-resize -w $WID 100 100 100 100 \
                    select --instance xterm \
                    move-resize 100 100 100 100 \
                    move-resize 50 y w h \
                    move-resize x 50 w h \
                    move-resize x y 50 h \
                    move-resize x y w 50

exit $?
