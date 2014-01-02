cmake_minimum_required(VERSION 2.4)

FIND_LIBRARY( ORACLE_LIBRARY1 "clntsh"
  "/opt/oracle/lib"
)

IF( ORACLE_LIBRARY1 )

  SET(ORACLE_LIBRARIES ${ORACLE_LIBRARY1} ${ORACLE_LIBRARY2} )

  MESSAGE(STATUS "oracle oci found at ${ORACLE_LIBRARIES}" )

  SET (CORE_SOURCES
    adbl_oracle.c
    
    adbl_oracle_common.c
    adbl_oracle_connection.c
    adbl_oracle_transaction.c
    adbl_oracle_createsql.c
  )

  SET(QUOM_INCLUDE_DIR
    "../"
    "../../core"
  )

  SET(ORACLE_INCLUDE_DIR
    "/opt/oracle/include"
  )

  ADD_LIBRARY( adbl_oracle SHARED ${CORE_SOURCES} )

  INCLUDE_DIRECTORIES( ${QUOM_INCLUDE_DIR} ${ORACLE_INCLUDE_DIR} )

  TARGET_LINK_LIBRARIES( adbl_oracle adbl ${ORACLE_LIBRARIES} )
  
ELSE( ORACLE_LIBRARY1 )

  MESSAGE(WARNING "oracleclient was not found on your system" )

ENDIF( ORACLE_LIBRARY1 )

