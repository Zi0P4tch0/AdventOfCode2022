#!/bin/bash

export GC_DEBUG=gc-friendly
export GC_SLICE=always-malloc
valgrind --tool=memcheck --leak-check=full --num-callers=20 "$@"
