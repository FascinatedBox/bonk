#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    get-title \
                    get-title --wait -w $WID %0 \
                    select --all get-title %@

exit $?
