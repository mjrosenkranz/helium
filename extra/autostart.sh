#!/bin/sh
export HELIUMPID="$(pidof helium)"
sxhkd -c $HOME/docs/code/helium/extra/sxhkdrc &
