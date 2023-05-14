#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
./bonk select --instance xterm \
                    decoration none \
                    decoration -w $WID resize,maximize,minimize,close \
                    decoration all

exit $?
