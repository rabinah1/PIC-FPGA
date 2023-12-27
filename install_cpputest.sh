#!/bin/bash

SCRIPT_DIR=$(cd $(dirname "$0") && pwd)

git clone https://github.com/cpputest/cpputest.git
cd ${SCRIPT_DIR}/cpputest/build
cmake ..
make -j$(nproc)
make install
cd ../src/CppUTest
cp -r /usr/local/lib/* .
cd ${SCRIPT_DIR}
