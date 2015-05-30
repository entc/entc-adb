cmake_minimum_required(VERSION 2.4)

SET (CORE_SOURCES  
  adbl.h
  ocadbl.h

  adbl_module.inc

  adbl_manager.c
  adbl_attributes.c
  adbl_query.c
  adbl_constraint.c
  adbl_insert.c
  adbl_update.c
  adbl_delete.c
  adbl_structs.h
  adbl_security.c
  adbl_security.h
  adbl_table.h
  adbl_table.c
)

ADD_LIBRARY( adbl SHARED ${CORE_SOURCES} )

INCLUDE_DIRECTORIES( ${ENTC_INCLUDE_DIRS} )

TARGET_LINK_LIBRARIES( adbl ${ENTC_LIBRARIES} )

set_target_properties(adbl PROPERTIES VERSION 1.0.5 SOVERSION 1.0.5)

