cmake_minimum_required(VERSION 2.4)

PROJECT(entc-base)

SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_MACOSX_RPATH 0)
SET(CMAKE_INSTALL_PREFIX /opt/entc_1_4/)

SET( SYSTEM_LIBRARIES )

find_package(OpenSSL)

#************* PTHREAD LIBRARY ***************************

IF( DISSABLE_EXTERN_PTHREAD )

  MESSAGE(STATUS "external pthread library was disabled" )

ELSE( DISSABLE_EXTERN_PTHREAD )

  FIND_LIBRARY( PTHREAD_LIBRARIES NAMES pthread
    HINTS
    "/usr/lib/"
    "/usr/local/lib/"
    "/opt/local/lib/"
  )

  IF( PTHREAD_LIBRARIES )

    MESSAGE(STATUS "pthread library found at ${PTHREAD_LIBRARIES}" )
  
    SET( SYSTEM_LIBRARIES ${SYSTEM_LIBRARIES} ${PTHREAD_LIBRARIES} )

  ENDIF( PTHREAD_LIBRARIES )

ENDIF( DISSABLE_EXTERN_PTHREAD )

#************* LD LIBRARY ***************************

FIND_LIBRARY( DL_LIBRARIES NAMES dl      
  HINTS
  "/usr/lib/"
  "/usr/local/lib/"
  "/opt/local/lib/"
)

IF( DL_LIBRARIES )

  MESSAGE(STATUS "dl library found at ${DL_LIBRARIES}" )          

  SET( SYSTEM_LIBRARIES ${SYSTEM_LIBRARIES} ${DL_LIBRARIES} )

ENDIF( DL_LIBRARIES )  

MESSAGE(STATUS "configured with system libraries: ${SYSTEM_LIBRARIES}" )


#************* LD LIBRARY ***************************

FIND_LIBRARY( DL_LIBRARIES NAMES dl      
  HINTS
  "/usr/lib/"
  "/usr/local/lib/"
  "/opt/local/lib/"
)

IF( DL_LIBRARIES )

  MESSAGE(STATUS "dl library found at ${DL_LIBRARIES}" )          

  SET( SYSTEM_LIBRARIES ${SYSTEM_LIBRARIES} ${DL_LIBRARIES} )

ENDIF( DL_LIBRARIES )  

MESSAGE(STATUS "configured with system libraries: ${SYSTEM_LIBRARIES}" )

SET (ENTC_SYS_HEADERS

  system/types.h
  system/ecdefs.h
  system/macros.h
  system/platform.h
  system/ecfile.h
  system/ecmutex.h
  system/ecthread.h
  system/ecdl.h
  system/ecdlib.h
  system/ectime.h
  system/ecexec.h
  system/ecrefcnt.h
  system/ecproc.h

)

SET (ENTC_SYS_SOURCES

  system/ecfile.c
  system/ecfile_win.c
  system/ecfile_unix.c
  system/ecfile_dos.c
  system/ecmutex_win.c
  system/ecmutex_unix.c
  system/ecmutex_dos.c
  system/ecthread_win.c
  system/ecthread_unix.c
  system/ecthread_dos.c
  system/ecdl.c
  system/ecdlib.c
  system/ectime.c
  system/ecexec_unix.c
  system/ecexec_win.c
  system/ecrefcnt.c
  system/ecproc_win.c
  system/ecproc_nix.c

)

SET (ENTC_AIO_HEADERS

  aio/ecaio.h
  aio/ecaio_ctx.h
  aio/ecaio_notify.h
  aio/ecaio_file.h
  aio/ecaio_socket.h
  aio/ecaio_sendf.h
  aio/ecaio_proc.h
  aio/ecaio_event.h
  aio/ecaio_msg.h
)

SET (ENTC_AIO_SOURCES

  aio/ecaio.c
  aio/ecaio_ctx.c
  aio/ecaio_notify.c
  aio/ecaio_file.c
  aio/ecaio_socket.c
  aio/ecaio_sendf.c
  aio/ecaio_proc.c
  aio/ecaio_event.c
  aio/ecaio_msg.c
)

SET (ENTC_TYPES_HEADERS

  types/ecerr.h
  types/ecalloc.h
  types/eclist.h
  types/ecmap.h
  types/ecstring.h
  types/ecbuffer.h
  types/ecstream.h
  types/eccursor.h
  types/ecudc.h
  types/ecstack.h
  types/ectable.h
)

SET (ENTC_TYPES_SOURCES

  types/ecerr.c
  types/ecalloc.c
  types/ecarray.c
  types/eclist.c
  types/ecmap.c
  types/ecstring.c
  types/ecbuffer.c
  types/ecstream.c
  types/eccursor.c
  types/ecudc.c
  types/ecstack.c
  types/ectable.c
  types/md5.c
)

SET (ENTC_TOOLS_HEADERS

  tools/eclog.h
  tools/ecxmlstream.h
  tools/ecjson.h
  tools/ecmime.h
  tools/eclparser.h
  tools/ecjparser.h
  tools/ectokenizer.h
  tools/ectemplate.h
  tools/eccrypt.h
  tools/ecreadbuffer.h
)

SET (ENTC_TOOLS_SOURCES

  tools/eclog.c
  tools/ecxmlstream.c
  tools/ecjson.c
  tools/ecmime.c
  tools/eclparser.c
  tools/ecjparser.c
  tools/ectokenizer.c
  tools/ectemplate.c
  tools/eccrypt.c
  tools/ecreadbuffer.c
)

SET (CORE_SOURCES_TESTS
  tests/ecenv.h
  tests/ecenv.c
)

INCLUDE_DIRECTORIES("." ${OPENSSL_INCLUDE_DIR})

source_group("System" FILES ${ENTC_SYS_SOURCES} ${ENTC_SYS_HEADERS})
source_group("AIO" FILES ${ENTC_AIO_SOURCES} ${ENTC_AIO_HEADERS})
source_group("Types" FILES ${ENTC_TYPES_SOURCES} ${ENTC_TYPES_HEADERS})
source_group("Tools" FILES ${ENTC_TOOLS_SOURCES} ${ENTC_TOOLS_HEADERS})
source_group("Tests" FILES ${CORE_SOURCES_TESTS})

ADD_LIBRARY( entc SHARED ${ENTC_SYS_SOURCES} ${ENTC_TYPES_HEADERS} ${ENTC_AIO_SOURCES} ${ENTC_AIO_HEADERS} ${ENTC_TYPES_SOURCES} ${ENTC_TOOLS_SOURCES} ${ENTC_TOOLS_HEADERS} ${CORE_SOURCES_TESTS})

TARGET_LINK_LIBRARIES( entc ${SYSTEM_LIBRARIES} ${OPENSSL_LIBRARIES})

set_target_properties(entc PROPERTIES VERSION 1.4.0 SOVERSION 1.4.0)

install (TARGETS entc DESTINATION lib)
install (FILES ${ENTC_SYS_HEADERS} DESTINATION include/entc/system)
install (FILES ${ENTC_AIO_HEADERS} DESTINATION include/entc/aio)
install (FILES ${ENTC_TYPES_HEADERS} DESTINATION include/entc/types)
install (FILES ${ENTC_TOOLS_HEADERS} DESTINATION include/entc/tools)

INSTALL_FILES (/${CMAKE_ROOT}/Modules/ FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindEntc_1_4.cmake)

#------------------------------------------------------------------------------

find_package(Lua2 QUIET)

IF(LUA_LIBRARIES)

MESSAGE(STATUS "lua library found at ${LUA_LIBRARIES}" )

INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})

SET (CORE_SOURCES_TESTAPP
  tests/eclua.h
  tests/eclua.c
  tests/ectest_app.c
)

ADD_EXECUTABLE( ectestapp ${CORE_SOURCES_TESTAPP} )
TARGET_LINK_LIBRARIES( ectestapp entc ${LUA_LIBRARIES})

ENDIF(LUA_LIBRARIES )

#------------------------------------------------------------------------------

SUBDIRS(tests)

