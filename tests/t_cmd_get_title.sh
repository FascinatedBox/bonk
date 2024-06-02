#!/usr/bin/env bash

xeyes 2> /dev/null &
WID=`./bonk select --retry --instance xeyes`

xtrace -D :9 ./bonk select --instance xeyes \
                    get-title \
                    get-title --wait -w $WID %0

pkill -9 xeyes
exit $?
