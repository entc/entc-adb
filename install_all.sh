#!/bin/bash

rm -Rf build_entc
mkdir build_entc

cd build_entc

cmake --configure ../entc 
cmake --build . --target install

cd ..

rm -Rf build_adbl
mkdir build_adbl

cd build_adbl

rm ../adbl/CMakeCache.txt
cmake --configure ../adbl
cmake --build . --target install

cd ..

rm -Rf build_adbo
mkdir build_adbo

cd build_adbo

cmake --configure ../adbo
cmake --build . --target install

cd ..
