#!/bin/bash

SCRIPT_DIR=$(cd $(dirname "$0") && pwd)

git clone https://github.com/cpputest/cpputest.git
cd ${SCRIPT_DIR}/cpputest
git checkout 81eb8b8fd3ba4a85097c01d1e114c4315b8d8b44
cd ${SCRIPT_DIR}/cpputest/build
cmake ..
make -j$(nproc)
make install
cd ../src/CppUTest
cp -r /usr/local/lib/* .
cd ${SCRIPT_DIR}
