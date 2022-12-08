#!/bin/bash

[ "$(uname)" != "Linux" ] && {
    echo "This script is for Linux only"
    exit 1
}

export GC_DEBUG=gc-friendly
export GC_SLICE=always-malloc
valgrind --tool=memcheck --leak-check=full --num-callers=20 "$@"
