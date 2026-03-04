#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <path> <dot_filename>"
    echo "Arguments: <path> # PATH_TO_SOURCE"
    echo "           <str>  # DOT_FILENAME"
    exit 1
fi

PATH_TO_PLUGIN="./install/lib/libLLVMPassDumper.so"
PATH_TO_SOURCE="$1"
PATH_TO_DOT="$2.dot"

clang -Xclang -load -Xclang ${PATH_TO_PLUGIN} \
      -fpass-plugin=${PATH_TO_PLUGIN} \
      -mllvm -dumper-pass-dot-out=${PATH_TO_DOT} \
      ${PATH_TO_SOURCE} -O2 -o ./build/a.out
