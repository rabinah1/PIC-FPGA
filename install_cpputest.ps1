cd $PSScriptRoot/cpputest/build
cmake ..
make
make install
cd ../src/CppUTest
cp -r C:\cygwin64\usr\local\lib\* .
cd $PSScriptRoot
