#!/bin/bash

clang -fpass-plugin=./libDumpPass.so c_examples/hello.c -O2 > dump.log
