#!/usr/bin/env bash

xtrace -D :9 ./bonk get-focus
xtrace -D :9 ./bonk get-focus activate
./bonk get-focus -h
exit $?
