cmake_minimum_required(VERSION 2.4)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
SET(CMAKE_MACOSX_RPATH 0)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_BUILD_TYPE Debug)

find_package(Entc_1_4 REQUIRED)
find_package(SQLiteAdbl QUIET)

IF(SQLITE_FOUND)

  MESSAGE(STATUS "sqlite found at ${SQLITE_LIBRARIES}" )

  SET (CORE_SOURCES
    adbl_sqlite3.c
  )

  ADD_LIBRARY( adbl_sqlite3 SHARED ${CORE_SOURCES} )

  INCLUDE_DIRECTORIES( "../core" ${SQLITE_INCLUDE_DIR} ${ENTC_INCLUDE_DIRS} )

  TARGET_LINK_LIBRARIES( adbl_sqlite3 adbl ${LIBXML2_LIBRARIES} ${SQLITE_LIBRARIES} )

ELSE(SQLITE_FOUND)

  MESSAGE(STATUS "sqlite was not found at ${SQLITE_LIBRARIES}: use included version" )

  SET (CORE_SOURCES
    adbl_sqlite3.c
    sqlite3.c
  )

  ADD_LIBRARY( adbl_sqlite3 SHARED ${CORE_SOURCES} )

  INCLUDE_DIRECTORIES( "../core" ${ENTC_INCLUDE_DIRS} )

  TARGET_LINK_LIBRARIES( adbl_sqlite3 ${ENTC_LIBRARIES} adbl )

ENDIF(SQLITE_FOUND)

install (TARGETS adbl_sqlite3 DESTINATION modules_adbl)

