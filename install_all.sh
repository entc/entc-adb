#!/bin/bash

echo "============ ENTC ============="

rm -Rf build_entc
mkdir build_entc

cd build_entc

rm ../entc/CMakeCache.txt
cmake --configure ../entc 
cmake --build . --target install

cd ..

echo "============ ADBL ============="

rm -Rf build_adbl
mkdir build_adbl

cd build_adbl

rm ../adbl/CMakeCache.txt
cmake --configure ../adbl
cmake --build . --target install

cd ..

echo "============ ADBO ============="

rm -Rf build_adbo
mkdir build_adbo

cd build_adbo

rm ../adbo/CMakeCache.txt
cmake --configure ../adbo
cmake --build . --target install

cd ..
