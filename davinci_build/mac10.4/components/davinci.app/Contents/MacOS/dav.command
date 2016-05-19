#!/bin/bash

base="/Applications/davinci.app"

which X11 > /dev/null 2>&1
[[ $? -eq 0 ]] && open -a X11

open -a Terminal
osascript -e "tell application \"Terminal\" to do script with command \"$base/Contents/MacOS/dav.start.2; exit $?\""

