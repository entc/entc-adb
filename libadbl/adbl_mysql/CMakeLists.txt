cmake_minimum_required(VERSION 2.4)

FIND_LIBRARY(MYSQL_LIBRARY
    NAMES mysqlclient
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}"/3rdParty/mysql/lib"
)

MARK_AS_ADVANCED(MYSQL_LIBRARY)

FIND_PATH(MYSQL_INCLUDE_DIR
    NAMES mysql.h
    PATHS "/opt/local/include" "/opt/local/include/mysql57" "/usr/include" "/usr/local/include" ${CMAKE_CURRENT_SOURCE_DIR}"/3rdParty/mysql/include"
    #PATH_SUFFIXES "mysql"
)

MARK_AS_ADVANCED(MYSQL_INCLUDE_DIR)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(MYSQL DEFAULT_MSG MYSQL_LIBRARY MYSQL_INCLUDE_DIR)

IF(MYSQL_FOUND)
    SET(MYSQL_LIBRARIES ${MYSQL_LIBRARY})
    SET(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})

    MESSAGE( STATUS "${MYSQL_INCLUDE_DIRS}")

    SET (CORE_SOURCES
        adbl_mysql.c
    )

    ADD_LIBRARY( adbl_mysql SHARED ${CORE_SOURCES} )

    INCLUDE_DIRECTORIES( "../core" ${MYSQL_INCLUDE_DIR} ${ENTC_INCLUDE_DIRS})

    TARGET_LINK_LIBRARIES( adbl_mysql adbl ${MYSQL_LIBRARIES} ${ENTC_LIBRARIES})

    ADD_EXECUTABLE( test_adbl_mysql_basic test_adbl_mysql_basic.c )
    TARGET_LINK_LIBRARIES( test_adbl_mysql_basic adbl_mysql ${ENTC_LIBRARIES})

    ADD_EXECUTABLE(test_adbl_mysql_ps test_adbl_mysql_ps.c )
    TARGET_LINK_LIBRARIES(test_adbl_mysql_ps adbl_mysql ${ENTC_LIBRARIES})

ELSE(MYSQL_FOUND)

  MESSAGE(WARNING "mysqlclient was not found on your system" )

ENDIF(MYSQL_FOUND)

