#!/bin/env sh
[ -z "$(pidof Xephyr)" ] && {
  /usr/bin/Xephyr -screen 800x600 -resizeable :1 &
}
