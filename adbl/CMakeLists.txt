cmake_minimum_required(VERSION 2.4)

set(CMAKE_MACOSX_RPATH 1)

PROJECT(adbl)

find_package(Entc_1_3)

MESSAGE(STATUS "+---- ADBL ----+")


add_subdirectory(core)

add_subdirectory(adbl_mysql)
add_subdirectory(adbl_sqlite)
#add_subdirectory(adbl_oracle)

