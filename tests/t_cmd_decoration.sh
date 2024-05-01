#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    decoration none \
                    decoration --wait close \
                    decoration -w $WID resize,maximize,minimize,close \
                    decoration all \
                    decoration -w $WID border,close,maximize,menu,minimize,resize,title \

exit $?
