cmake_minimum_required(VERSION 2.4)

PROJECT(entc-base)

SET(CMAKE_VERBOSE_MAKEFILE OFF)

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


cmake_minimum_required(VERSION 2.4)

ADD_DEFINITIONS(-ggdb)

SET (CORE_SOURCES_SYS
  system/types.h
  system/ecdefs.h
  system/macros.h
  system/platform.h
  system/ecfile.h
  system/ecfile.c
  system/ecfile_win.c
  system/ecfile_unix.c
  system/ecfile_dos.c
  system/ecmutex.h
  system/ecmutex_win.c
  system/ecmutex_unix.c
  system/ecmutex_dos.c
  system/ecsocket.h
  system/ecsocket_win.c
  system/ecsocket_unix.c
  system/ecsocket_dos.c
  system/ecevents.h
  system/ecevents_win.c
  system/ecevents_bsd.c
  system/ecevents_linux.c
  system/ecevents_dos.c
  system/ecthread.h
  system/ecthread_win.c
  system/ecthread_unix.c
  system/ecthread_dos.c
  system/ecsignal.h
  system/ecsignal_win.c
  system/ecsignal_unix.c
  system/ecsignal_dos.c
  system/ecdl.h
  system/ecdl.c
  system/ecdlib.h
  system/ecdlib.c
  system/ectime.h
  system/ectime.c
  system/ecexec.h
  system/ecexec_unix.c
  system/ecexec_win.c
  system/ecrefcnt.h
  system/ecrefcnt.c
)

SET (CORE_SOURCES_TYPES
  types/ecerr.h
  types/ecerr.c
  types/ecalloc.h
  types/ecalloc.c
  types/ecarray.c
types/ecintmap.h
types/ecintmap.c
types/eclist.h
types/eclist.c
types/ecmap.h
types/ecmap.c
types/ecmapchar.h
types/ecmapchar.c
types/ecstack.c
  types/ectable.c
  types/ecchain.c
  types/ecchain.h
  types/ecstring.h
  types/ecstring.c
  types/ecbuffer.h
  types/ecbuffer.c
  types/ocstring.h
  types/ecstream.h
  types/ecstream.c
  types/eccursor.h
  types/eccursor.c
  types/ecudc.h
  types/ecudc.c
  
  types/md5.c
)

SET (CORE_SOURCES_UTILS
  utils/ecmessages.h
  utils/ecmessages.c
  utils/eclogger.h
  utils/eclogger.c
  utils/ecreadbuffer.h
  utils/ecreadbuffer.c
  utils/ecsmartbuffer.h
  utils/ecsmartbuffer.c
  utils/ecstreambuffer.h
  utils/ecstreambuffer.c
  utils/ecobserver.h
  utils/ecobserver.c
  utils/ecsecfile.h
  utils/ecsecfile.c
)

SET (CORE_SOURCES_TOOLS
  tools/echttp.h
  tools/echttp.c
  tools/echtml.h
  tools/echtml.c
  tools/ecxmlstream.h
  tools/ecxmlstream.c
  tools/ecjson.h
  tools/ecjson.c
  tools/ecmime.h
  tools/ecmime.c
  tools/eclparser.h
  tools/eclparser.c
  tools/ectokenizer.h
  tools/ectokenizer.c

  tools/ectemplate.h
  tools/ectemplate.c
)

SET (CORE_SOURCES_TESTS
  tests/ecenv.h
  tests/ecenv.c
)

INCLUDE_DIRECTORIES("." ${OPENSSL_INCLUDE_DIR})

source_group("System" FILES ${CORE_SOURCES_SYS})
source_group("Types" FILES ${CORE_SOURCES_TYPES})
source_group("Utils" FILES ${CORE_SOURCES_UTILS})
source_group("Tools" FILES ${CORE_SOURCES_TOOLS})
source_group("Tests" FILES ${CORE_SOURCES_TESTS})

ADD_LIBRARY( entc SHARED ${CORE_SOURCES_SYS} ${CORE_SOURCES_TYPES} ${CORE_SOURCES_UTILS} ${CORE_SOURCES_TOOLS} ${CORE_SOURCES_TESTS})

TARGET_LINK_LIBRARIES( entc ${SYSTEM_LIBRARIES} ${OPENSSL_LIBRARIES})

set_target_properties(entc PROPERTIES VERSION 1.3.0 SOVERSION 1.3.0)

#------------------------------------------------------------------------------

find_package(Lua)

IF(LUA_LIBRARIES)

MESSAGE(STATUS "lua library found at ${LUA_LIBRARIES}" )

INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})

ADD_EXECUTABLE( ectestapp tests/ectest_app.c )
TARGET_LINK_LIBRARIES( ectestapp entc )

ENDIF(LUA_LIBRARIES )

#------------------------------------------------------------------------------

SUBDIRS(tests)

