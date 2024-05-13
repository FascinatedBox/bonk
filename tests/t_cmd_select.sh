#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --show --instance xterm \
                    select --if-empty-stack --show --instance banana \
                    select --show --exact-class XTerm \
                    select --has-property WM_NAME \
                    select --retry --instance xterm \
                    select --show --exact-title unfindable \
                    select --show --title xterm \
                    select --show --title xeyes \
                    select --show --pid 1 \
                    select --all --limit 1

exit $?
