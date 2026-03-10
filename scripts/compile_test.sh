#!/bin/bash

if [ $# -ne 3 ]; then
    echo "Usage: $0 <path> <dot_filename>"
    echo "Arguments: <path> # PATH_TO_SOURCE"
    echo "           <str>  # DUMP_BEFORE_OPT_DOT_FILENAME"
    echo "           <str>  # DUMP_AFTER_OPT_DOT_FILENAME"
    exit 1
fi

PATH_TO_PLUGIN="./install/lib/libLLVMIRDumper.so"
PATH_TO_SOURCE="$1"
DUMP_BEFORE_OPT_DOT_FILENAME="$2.dot"
DUMP_AFTER_OPT_DOT_FILENAME="$3.dot"

clang -Xclang -load -Xclang ${PATH_TO_PLUGIN} \
      -fpass-plugin=${PATH_TO_PLUGIN} \
      -mllvm -dumper-pass-dot-out-before-opt=${DUMP_BEFORE_OPT_DOT_FILENAME} \
      -mllvm -dumper-pass-dot-out-after-opt=${DUMP_AFTER_OPT_DOT_FILENAME} \
      ${PATH_TO_SOURCE} -O2 -o ./build/a.out
