#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    unmap \
                    unmap --wait -w $WID %0 \
                    select --all --instance xterm \
                    map \
                    map --wait -w $WID %0
exit $?
