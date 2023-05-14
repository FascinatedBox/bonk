#!/usr/bin/env bash

xtrace -D :9 ./bonk get-active
xtrace -D :9 ./bonk get-active activate
./bonk get-active -h
exit $?
