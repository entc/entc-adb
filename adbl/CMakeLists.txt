cmake_minimum_required(VERSION 2.4)

PROJECT(adbl)

SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_MACOSX_RPATH 0)
SET(CMAKE_INSTALL_PREFIX /opt/entc_1_4/)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_BUILD_TYPE Debug)

find_package(Entc_1_4 REQUIRED)

SET (ADBL_HEADERS

  core/adbl.h
  core/adbl_manager.h
  core/adbl_attributes.h
  core/adbl_query.h
  core/adbl_constraint.h
  core/adbl_insert.h
  core/adbl_update.h
  core/adbl_delete.h
  core/adbl_structs.h
  core/adbl_security.h
  core/adbl_table.h
  core/adbl_procedure.h
)

SET (ADBL_SOURCES

  core/adbl_module.inc
  core/adbl_manager.c
  core/adbl_attributes.c
  core/adbl_query.c
  core/adbl_constraint.c
  core/adbl_insert.c
  core/adbl_update.c
  core/adbl_delete.c
  core/adbl_security.c
  core/adbl_table.c
  core/adbl_procedure.c
)

ADD_LIBRARY( adbl SHARED ${ADBL_SOURCES} ${ADBL_HEADERS} )

INCLUDE_DIRECTORIES( ${ENTC_INCLUDE_DIRS} )

TARGET_LINK_LIBRARIES( adbl ${ENTC_LIBRARIES} )

set_target_properties(adbl PROPERTIES VERSION 1.4.0 SOVERSION 1.4.0)
set_property(TARGET adbl PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

install (TARGETS adbl DESTINATION lib)
install (FILES ${ADBL_HEADERS} DESTINATION include/adbl)

INSTALL_FILES (/${CMAKE_ROOT}/Modules/ FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindAdbl_1_4.cmake)

add_subdirectory(adbl_mysql)
add_subdirectory(adbl_sqlite)
#add_subdirectory(adbl_oracle)
