rd /s /q "build_entc"
mkdir build_entc

del entc\CMakeCache.txt

cd build_entc
cmake --configure ../entc
cmake --build . --target install
cd ..

rd /s /q "build_adbl"
mkdir build_adbl

del adbl\CMakeCache.txt

cd build_adbl
cmake --configure ../adbl
cmake --build . --target install
cd ..

rd /s /q "build_adbo"
mkdir build_adbo

del adbo\CMakeCache.txt

cd build_adbo
cmake --configure ../adbo
cmake --build . --target install
cd ..
