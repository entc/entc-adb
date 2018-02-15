cmake_minimum_required(VERSION 2.4)

SET (ADBL_HEADERS

  adbl.h
  adbl_manager.h
  adbl_attributes.h
  adbl_query.h
  adbl_insert.h
  adbl_update.h
  adbl_delete.h
  adbl_structs.h
  adbl_security.h
  adbl_table.h
  adbl_procedure.h
)

SET (ADBL_SOURCES

  adbl_module.inc
  adbl_manager.c
  adbl_attributes.c
  adbl_query.c
  adbl_constraint.c
  adbl_insert.c
  adbl_update.c
  adbl_delete.c
  adbl_security.c
  adbl_table.c
  adbl_procedure.c
)

ADD_LIBRARY( adbl SHARED ${ADBL_SOURCES} ${ADBL_HEADERS} )

INCLUDE_DIRECTORIES( ${ENTC_INCLUDE_DIRS} )

TARGET_LINK_LIBRARIES( adbl ${ENTC_LIBRARIES} )

set_target_properties(adbl PROPERTIES VERSION 1.3.0 SOVERSION 1.3.0)

SET (CMAKE_INSTALL_PREFIX /opt/entc_1_3/)

install (TARGETS adbl DESTINATION lib)
install (FILES ${ADBL_HEADERS} DESTINATION include/adbl)

INSTALL_FILES (/${CMAKE_ROOT}/Modules/ FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindAdbl_1_3.cmake)
