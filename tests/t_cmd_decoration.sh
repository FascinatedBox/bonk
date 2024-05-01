#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    decoration none \
                    decoration --wait maximize \
                    decoration -w $WID resize,maximize,minimize,close \
                    decoration all

exit $?
