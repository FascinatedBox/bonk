#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
xtrace -D :9 ./bonk select --all --instance xterm \
                    state -a above \
                          -a below \
                          -a demands_attention \
                          -a fullscreen \
                          -a hidden \
                          -a maximized_horz \
                          -a maximized_vert \
                    state -w $WID \
                          -a modal \
                          -a shaded \
                          -a skip_pager \
                          -a skip_taskbar \
                          -a sticky
xtrace -D :9 ./bonk select --all --instance xterm \
                    state -r above \
                          -r below \
                          -r demands_attention \
                          -r fullscreen \
                          -r hidden \
                          -r maximized_horz \
                          -r maximized_vert \
                    state -w $WID \
                          -r modal \
                          -r shaded \
                          -r skip_pager \
                          -r skip_taskbar \
                          -r sticky
xtrace -D :9 ./bonk select --all --instance xterm \
                    state -t above \
                          -t below \
                          -t demands_attention \
                          -t fullscreen \
                          -t hidden \
                          -t maximized_horz \
                          -t maximized_vert \
                    state -w $WID \
                          -t modal \
                          -t shaded \
                          -t skip_pager \
                          -t skip_taskbar \
                          -t sticky
xtrace -D :9 ./bonk select --all --instance xterm \
                    state -t above \
                          -t below \
                          -t demands_attention \
                          -t fullscreen \
                          -t hidden \
                          -t maximized_horz \
                          -t maximized_vert \
                    state -w $WID \
                          -t modal \
                          -t shaded \
                          -t skip_pager \
                          -t skip_taskbar \
                          -t sticky
exit $?
