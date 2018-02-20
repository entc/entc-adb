rd /s /q "build_entc"
mkdir build_entc
cd build_entc

cmake --configure ../entc
cmake --build . --target install

cd ..

rd /s /q "build_adbl"
mkdir build_adbl
cd build_adbl

cmake --configure ../adbl
cmake --build . --target install

cd ..

rd /s /q "build_adbo"
mkdir build_adbo
cd build_adbo

cmake --configure ../adbo
cmake --build . --target install

cd ..
