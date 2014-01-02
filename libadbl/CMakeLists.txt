cmake_minimum_required(VERSION 2.4)

PROJECT(adbl)

find_package(Entc)

add_subdirectory(core)

add_subdirectory(adbl_mysql)
add_subdirectory(adbl_sqlite)
#add_subdirectory(adbl_oracle)
