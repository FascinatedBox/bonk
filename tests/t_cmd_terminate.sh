#!/usr/bin/env bash

WID=`./bonk select --has-property _XKB_RULES_NAMES`
xtrace -D :9 ./bonk select --has-property _XKB_RULES_NAMES \
                    terminate \
                    terminate --wait -w $WID %0
exit $?
