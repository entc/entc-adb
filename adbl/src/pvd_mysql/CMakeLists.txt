cmake_minimum_required(VERSION 2.4)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/modules)
SET(CMAKE_MACOSX_RPATH 0)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_BUILD_TYPE Debug)

find_package(AdblMysql )

IF(MYSQL_FOUND)

  INCLUDE_DIRECTORIES( "../core" ${MYSQL_INCLUDES})

  SET (CORE_SOURCES
    adbl_mysql.c
    bindvars.c
    prepare.c
  )

  ADD_LIBRARY(adbl_mysql SHARED ${CORE_SOURCES} )
  TARGET_LINK_LIBRARIES( adbl_mysql adbl cape ${MYSQL_LIBRARIES})

  #--------------------------------------------------------------
  # unit tests 
  
  INCLUDE_DIRECTORIES( "." ${MYSQL_INCLUDES})

  add_executable          (ut_basic "tests/ut_basic.c")
  target_link_libraries   (ut_basic adbl_mysql)

  
ELSE(MYSQL_FOUND)

  MESSAGE(WARNING "mysqlclient was not found on your system" )

ENDIF(MYSQL_FOUND)
