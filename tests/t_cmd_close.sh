#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
./bonk select --instance xterm \
                    close \
                    close --wait -w $WID

exit $?
