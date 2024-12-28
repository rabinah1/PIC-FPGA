#!/bin/bash

SCRIPT_DIR=$(cd $(dirname "$0") && pwd)

git clone https://github.com/cpputest/cpputest.git ${SCRIPT_DIR}/cpputest
cd ${SCRIPT_DIR}/cpputest
git checkout 81eb8b8fd3ba4a85097c01d1e114c4315b8d8b44
cd ${SCRIPT_DIR}/cpputest/build
cmake ..
make -j$(nproc)
cp ./src/CppUTestExt/libCppUTestExt.a ../src/CppUTest
cp ./src/CppUTest/libCppUTest.a ../src/CppUTest
cd ${SCRIPT_DIR}
