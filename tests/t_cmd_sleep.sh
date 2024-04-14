#!/usr/bin/env bash

# Sleep code copied from GNU Bash is assumed to work, but make sure.
xtrace -D :9 ./bonk sleep .1s \
                    sleep .01m.1s \
                    sleep 0
exit $?
