#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    lower \
                    lower --wait -w $WID %0
exit $?
