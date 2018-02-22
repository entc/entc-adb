cmake_minimum_required(VERSION 2.4)

PROJECT(adbo)

SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_MACOSX_RPATH 0)
SET(CMAKE_INSTALL_PREFIX /opt/entc_1_4/)

find_package(Entc_1_4)
find_package(Adbl_1_4)

SET (ADBO_HEADERS
  core/adbo_types.h
  core/adbo_context.h
  core/adbo_value.h
  core/adbo_data.h
  core/adbo.h
)

SET (ADBO_SOURCES
  core/adbo_context_intern.h
  core/adbo_context.c
  core/adbo_node.c
  core/adbo_value.c
  core/adbo_item.c
  core/adbo_data.c
  core/adbo.c
)

INCLUDE_DIRECTORIES( ${ENTC_INCLUDE_DIRS} ${ADBL_INCLUDE_DIRS} )

ADD_LIBRARY( adbo SHARED ${ADBO_HEADERS} ${ADBO_SOURCES})
TARGET_LINK_LIBRARIES( adbo ${ENTC_LIBRARIES} ${ADBL_LIBRARIES} )

set_target_properties(adbo PROPERTIES VERSION 1.4.0 SOVERSION 1.4.0)

install (TARGETS adbo DESTINATION lib)
install (FILES ${ADBO_HEADERS} DESTINATION include/adbo)

INSTALL_FILES (/${CMAKE_ROOT}/Modules/ FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindAdbo_1_4.cmake)
