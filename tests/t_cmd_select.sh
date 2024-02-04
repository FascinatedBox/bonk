#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --show --instance xterm \
                    select --if-empty-stack --show --instance banana \
                    select --show --exact-class XTerm \
                    select --show --exact-classname xterm \
                    select --show --classname xterm \
                    select --has-property WM_NAME \
                    select --sync --instance xterm \
                    select --retry --instance xterm \
                    select --show --exact-title xterm \
                    select --show --title xterm \
                    select --show --title xeyes

exit $?
