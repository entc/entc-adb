cmake_minimum_required(VERSION 2.4)

FIND_LIBRARY( SQLITE_LIBRARIES NAMES "sqlite3"
  HINTS 
  "lib/adbl/adbl_sqlite"
  "/opt/local/lib/"
)

IF( SQLITE_LIBRARIES )

  MESSAGE(STATUS "sqlite found at ${SQLITE_LIBRARIES}" )

  SET (CORE_SOURCES
    adbl_sqlite3.c
  )

  ADD_LIBRARY( adbl_sqlite3 SHARED ${CORE_SOURCES} )

  INCLUDE_DIRECTORIES( "../core" ${SQLITE_INCLUDE_DIR} ${ENTC_INCLUDE_DIRS} )

  TARGET_LINK_LIBRARIES( adbl_sqlite3 adbl ${LIBXML2_LIBRARIES} ${SQLITE_LIBRARIES} )

ELSE( SQLITE_LIBRARIES )

  MESSAGE(STATUS "sqlite was not found at ${SQLITE_LIBRARIES}: use included version" )

  SET (CORE_SOURCES
    adbl_sqlite3.c
    sqlite3.c
  )

  ADD_LIBRARY( adbl_sqlite3 SHARED ${CORE_SOURCES} )

  INCLUDE_DIRECTORIES( ${ENTC_INCLUDE_DIRS} )

  TARGET_LINK_LIBRARIES( adbl_sqlite3 ${ENTC_LIBRARIES} adbl )

ENDIF( SQLITE_LIBRARIES )

