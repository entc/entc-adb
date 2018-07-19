if (NOT SQLITE_FOUND)

  ## try the default way
  find_package(SQLite QUIET)
  
  if (NOT SQLITE_FOUND)

    ##____________________________________________________________________________
    ## Check for the header files

    find_path (SQLITE_INCLUDES
      NAMES sqlite3.h
      HINTS ${CMAKE_INSTALL_PREFIX}
    )

    ##____________________________________________________________________________
    ## Check for the library

    find_library (SQLITE_LIBRARIES sqlite3 
      HINTS ${CMAKE_INSTALL_PREFIX}
    )

    ##____________________________________________________________________________
    ## Actions taken when all components have been found

    find_package_handle_standard_args (SQLITE DEFAULT_MSG SQLITE_LIBRARIES SQLITE_INCLUDES)

    if (SQLITE_FOUND)
      if (NOT SQLITE_FIND_QUIETLY)
        message (STATUS "SQLITE_INCLUDES  = ${SQLITE_INCLUDES}")
        message (STATUS "SQLITE_LIBRARIES = ${SQLITE_LIBRARIES}")
      endif (NOT SQLITE_FIND_QUIETLY)
    else (SQLITE_FOUND)
      if (SQLITE_FIND_REQUIRED)
        message (FATAL_ERROR "Could not find SQLITE!")
      endif (SQLITE_FIND_REQUIRED)
    endif (SQLITE_FOUND)

    ##____________________________________________________________________________
    ## Mark advanced variables

    mark_as_advanced (SQLITE_ROOT_DIR SQLITE_INCLUDES SQLITE_LIBRARIES)

  endif (NOT SQLITE_FOUND)
    
endif (NOT SQLITE_FOUND)
