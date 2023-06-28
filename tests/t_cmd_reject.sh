#!/usr/bin/env bash

xtrace -D :9 ./bonk select --all \
                    reject --show --exact-class XTerm \
                    select --all \
                    reject --show --exact-classname xterm \
                    select --all \
                    reject --show --classname xterm \
                    select --all \
                    reject --show --instance xterm \
                    select --all \
                    reject --show --exact-instance xterm \
                    select --all \
                    reject --has-property WM_NAME \
                    select --all \
                    reject --show --exact-title xterm \
                    select --all \
                    reject --has-state ABOVE \
                    reject --show --title xterm \
                    reject --show --title xeyes

exit $?
