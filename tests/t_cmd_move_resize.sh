#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk move-resize -w $WID 100 100 100 100 \
                    select --instance xterm \
                    move-resize 100 100 100 100

exit $?
