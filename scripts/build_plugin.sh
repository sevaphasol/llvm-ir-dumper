#!/bin/bash

cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build -j$(nproc)
cmake --install build --prefix install
ln -sf build/compile_commands.json
