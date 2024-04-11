#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --instance xterm \
                    prop-adjust --class xterm- \
                    prop-adjust --wait --class xterm \
                    prop-adjust --instance xterm- \
                    prop-adjust --wait --instance xterm \
                    prop-adjust --title xterm- \
                    prop-adjust --wid $WID --wait --title xterm

xtrace -D :9 ./bonk select --instance xterm \
                    prop-delete --wait WM_HINTS \
                    prop-delete -w $WID WM_HINTS \

exit $?
