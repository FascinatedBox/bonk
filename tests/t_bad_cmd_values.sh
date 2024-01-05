#!/usr/bin/env bash

WID=`./bonk select --instance xterm`
./bonk decoration -w $WID bad-name && exit 99
./bonk move-resize -w $WID 100 100 -100 -100 && exit 99
./bonk move-resize -w $WID a 100 100 100 && exit 99
./bonk move-resize -w $WID 100 b 100 100 && exit 99
./bonk move-resize -w $WID 100 100 c 100 && exit 99
./bonk move-resize -w $WID 100 100 100 d && exit 99
./bonk move-resize -w $WID 100asdf 100 100 d && exit 99
./bonk select --quickbrownfox && exit 99
./bonk select --has-state asdf && exit 99
./bonk select --all reject --quickbrownfox && exit 99
./bonk select --all reject --has-state asdf && exit 99
./bonk decoration -w $WID && exit 99
exit 0
