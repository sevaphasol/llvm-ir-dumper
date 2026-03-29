#!/bin/bash

cmake -B build -DGRAPHCC_INSTALL_SYMLINK=ON -DGRAPHCC_INSTALL_SYMLINK_DIR=$(pwd)
cmake --build build -j$(nproc)
cmake --install build --prefix install
