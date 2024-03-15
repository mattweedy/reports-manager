#!/bin/bash

case "$1" in
    start)
        ./daemon
        ;;
    stop)
        killall -e daemon
        ;;
    *)
        echo "Usage: $0 {start|stop}"
esac