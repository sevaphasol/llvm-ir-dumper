#!/bin/bash

cmake -B build -DGRAPHCC_INSTALL_SYMLINK=OFF
cmake --build build -j$(nproc)
cmake --install build --prefix install
ln -sf build/compile_commands.json
