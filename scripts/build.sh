#!/bin/bash

clang++ LLVM_Pass_dumper.cpp -fPIC -shared -I$(llvm-config --includedir) -o libDumpPass.so
