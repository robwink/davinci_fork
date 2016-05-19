#!/bin/bash

base="/Applications/davinci.app/Contents/Resources"

mypath=`pwd`
export EDITOR="/usr/bin/pico"
export DV_VIEWER="$base/ImageJ/ghetto_display.sh"
export GPLOT_CMD="/usr/local/bin/gnuplot"
export PATH=$PATH:$base/bin/
cd

open -a X11
open -a Terminal /usr/local/davinci/bin/davinci
