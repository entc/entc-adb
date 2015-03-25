cmake_minimum_required(VERSION 2.4)

FIND_LIBRARY( MYSQL_LIBRARIES "mysqlclient"
  "/usr/lib/mysql/"
  "/usr/local/mysql/lib/"
  "/usr/local/lib/mysql/"
  "/usr/local/mysql/lib/mysql"
  "/opt/local/lib/mysql5/mysql"
)

IF( MYSQL_LIBRARIES )

  MESSAGE(STATUS "mysqlclient found at ${MYSQL_LIBRARIES}" )

  SET (CORE_SOURCES
    adbl_mysql.c
  )

  SET(MYSQL_INCLUDE_DIR
    "/usr/include/mysql"
    "/opt/local/include/mysql5/mysql/"
    "/usr/local/mysql/include/"
  )

  ADD_LIBRARY( adbl_mysql SHARED ${CORE_SOURCES} )

  INCLUDE_DIRECTORIES( "../core" ${MYSQL_INCLUDE_DIR} ${ENTC_INCLUDE_DIRS})

  TARGET_LINK_LIBRARIES( adbl_mysql adbl ${MYSQL_LIBRARIES} ${ENTC_LIBRARIES})
  
#  ADD_EXECUTABLE( test_adbl_mysql_basic test_adbl_mysql_basic.c )
#  TARGET_LINK_LIBRARIES( test_adbl_mysql_basic adbl_mysql ${ENTC_LIBRARIES})

ELSE( MYSQL_LIBRARIES )

  MESSAGE(WARNING "mysqlclient was not found on your system" )

ENDIF( MYSQL_LIBRARIES )

