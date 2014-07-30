cmake_minimum_required(VERSION 2.4)

PROJECT(entc-base)

INCLUDE_DIRECTORIES( "../" )

source_group("Tests" FILES ${TESTS_SOURCES})

ADD_EXECUTABLE( unittest_exec system/unittest_exec.c )
TARGET_LINK_LIBRARIES( unittest_exec entc ) 

ADD_EXECUTABLE( unittest_asyncserv tools/unittest_asyncserv.c )
TARGET_LINK_LIBRARIES( unittest_asyncserv entc ) 
