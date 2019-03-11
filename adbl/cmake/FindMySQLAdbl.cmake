if (NOT MYSQL_FOUND)

  ## try the default way
  find_package(MySQL QUIET)
  
  if (NOT MYSQL_FOUND)

    ##____________________________________________________________________________
    ## Check for the header files

    find_path (MYSQL_INCLUDES
      NAMES mysql.h
      HINTS ${CMAKE_INSTALL_PREFIX} ${CMAKE_CURRENT_SOURCE_DIR}"/3rdParty/mysql/include" "/opt/local/include/mysql57"
      PATH_SUFFIXES mariadb
    )

    ##____________________________________________________________________________
    ## Check for the library

    find_library (MYSQL_LIBRARIES mariadb
      HINTS ${CMAKE_INSTALL_PREFIX} ${CMAKE_CURRENT_SOURCE_DIR}"/3rdParty/mysql/lib" "/opt/local/lib/mysql57/mysql"
    )

    ##____________________________________________________________________________
    ## Actions taken when all components have been found

    find_package_handle_standard_args (MYSQL DEFAULT_MSG MYSQL_LIBRARIES MYSQL_INCLUDES)

    if (MYSQL_FOUND)
      if (NOT MYSQL_FIND_QUIETLY)
        message (STATUS "MYSQL_INCLUDES  = ${MYSQL_INCLUDES}")
        message (STATUS "MYSQL_LIBRARIES = ${MYSQL_LIBRARIES}")
      endif (NOT MYSQL_FIND_QUIETLY)
    else (MYSQL_FOUND)
      if (MYSQL_FIND_REQUIRED)
        message (FATAL_ERROR "Could not find MYSQL!")
      endif (MYSQL_FIND_REQUIRED)
    endif (MYSQL_FOUND)

    ##____________________________________________________________________________
    ## Mark advanced variables

    mark_as_advanced (MYSQL_ROOT_DIR MYSQL_INCLUDES MYSQL_LIBRARIES)

  endif (NOT MYSQL_FOUND)
    
endif (NOT MYSQL_FOUND)
