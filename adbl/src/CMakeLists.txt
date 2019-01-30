cmake_minimum_required(VERSION 2.4)

SET(CMAKE_BUILD_TYPE Debug)

#----------------------------------------------------------------------------------

SET(ADBL_CORE_SOURCES
  core/adbl.c
)

SET(ADBL_CORE_HEADERS
  core/adbl.h
)

#----------------------------------------------------------------------------------

INCLUDE_DIRECTORIES("." "../../l_cape/src")

add_library             (adbl SHARED ${ADBL_CORE_SOURCES} ${ADBL_CORE_HEADERS})
target_link_libraries   (adbl cape)
set_target_properties   (adbl PROPERTIES VERSION 1.0.1 SOVERSION 1.0.1)

#----------------------------------------------------------------------------------

INCLUDE_DIRECTORIES("." "core")

SET(ADBL_APP_SOURCES
  app/adbl_app.c
)

add_executable          (adbl_app ${ADBL_APP_SOURCES})
target_link_libraries   (adbl_app adbl)
set_target_properties   (adbl_app PROPERTIES VERSION 1.0.1 SOVERSION 1.0.1)

#----------------------------------------------------------------------------------

SUBDIRS(pvd_mysql)


