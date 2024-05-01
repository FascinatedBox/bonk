#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    close \
                    close --wait -w $WID

exit $?
