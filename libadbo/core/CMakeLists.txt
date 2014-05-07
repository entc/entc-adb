cmake_minimum_required(VERSION 2.4)

SET (CORE_SOURCES  
  adbo_types.h
  adbo_context.h
  adbo_context_intern.h
  adbo_context.c
  adbo_schema.h
  adbo_schema.c
  adbo_object.h
  adbo_object.c
  adbo_node.h
  adbo_node.c
  adbo_value.h
  adbo_value.c
  adbo_item.h
  adbo_item.c
  adbo_container.h
  adbo_container.c
  adbo_substitute.h
  adbo_substitute.c
  
  adbo.h
  adbo.c
)

ADD_LIBRARY( adbo SHARED ${CORE_SOURCES} )

INCLUDE_DIRECTORIES( ${ENTC_INCLUDE_DIRS} ${ADBL_INCLUDE_DIRS} )

TARGET_LINK_LIBRARIES( adbo ${ENTC_LIBRARIES} ${ADBL_LIBRARIES} )

set_target_properties(adbo PROPERTIES VERSION 1.0.1 SOVERSION 1.0.1)

GET_TARGET_PROPERTY(BIN_NAME adbo LOCATION)
ADD_CUSTOM_COMMAND(
  TARGET adbo
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${BIN_NAME} ../tests
  COMMENT "Copy ${BIN_NAME}"
)