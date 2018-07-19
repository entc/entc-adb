cmake_minimum_required(VERSION 2.4)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
SET(CMAKE_MACOSX_RPATH 0)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_BUILD_TYPE Debug)

find_package(Entc_1_4 REQUIRED)
find_package(MySQLAdbl QUIET)

IF(MYSQL_FOUND)

  INCLUDE_DIRECTORIES( "../core" ${MYSQL_INCLUDES} ${ENTC_INCLUDE_DIRS})

  SET (CORE_SOURCES adbl_mysql.c)

  ADD_LIBRARY( adbl_mysql SHARED ${CORE_SOURCES} )

  TARGET_LINK_LIBRARIES( adbl_mysql adbl ${MYSQL_LIBRARIES} ${ENTC_LIBRARIES})

  ## --- tests ---
  
  ADD_EXECUTABLE( test_adbl_mysql_basic test_adbl_mysql_basic.c )
  TARGET_LINK_LIBRARIES( test_adbl_mysql_basic adbl_mysql ${ENTC_LIBRARIES})

  ADD_EXECUTABLE(test_adbl_mysql_ps test_adbl_mysql_ps.c )
  TARGET_LINK_LIBRARIES(test_adbl_mysql_ps adbl_mysql ${ENTC_LIBRARIES})

  install (TARGETS adbl_mysql DESTINATION modules_adbl)
  
  set_target_properties(adbl_mysql PROPERTIES VERSION 1.4.0 SOVERSION 1.4.0)
  set_property(TARGET adbl_mysql PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  
ELSE(MYSQL_FOUND)

  MESSAGE(WARNING "mysqlclient was not found on your system" )

ENDIF(MYSQL_FOUND)
