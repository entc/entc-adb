cmake_minimum_required(VERSION 2.4)

PROJECT(entc-base)

#------------------------------------------------------------------------------

SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_MACOSX_RPATH 0)
SET(CMAKE_INSTALL_PREFIX /opt/entc_1_4/)
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#------------------------------------------------------------------------------

find_package(Crypt REQUIRED)
find_package(PThread REQUIRED)
find_package(DL REQUIRED)

#------------------------------------------------------------------------------

SET (ENTC_SYSTEM_HEADERS

  system/platform.h
  system/ecfile.h
  system/ecdl.h
  system/ecdlib.h
  system/ectime.h
  system/ecexec.h
  system/ecrefcnt.h
  system/ecproc.h
  system/ecdaemon.h

)

SET (ENTC_SYSTEM_SOURCES

  system/ecfile.c
  system/ecfile_win.c
  system/ecfile_unix.c
  system/ecfile_dos.c
  system/ecdl.c
  system/ecdlib.c
  system/ectime.c
  system/ecexec_unix.c
  system/ecexec_win.c
  system/ecrefcnt.c
  system/ecproc_win.c
  system/ecproc_nix.c
  system/ecdaemon.c

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
  types/ecmap.c
  #types/ecmap_new2.c
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
  tools/eccode.h
  tools/ecreadbuffer.h
  tools/entc_multipart.h
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
  tools/eccode.c
  tools/ecreadbuffer.c
  tools/entc_multipart.c
)

SET (ENTC_TESTS_HEADERS
  tests/ecenv.h
)

SET(ENTC_TESTS_SOURCES
  tests/ecenv.c
)

#------------------------------------------------------------------------------

SET (ENTC_STC_HEADERS

  stc/entc_str.h
  stc/entc_list.h
  stc/entc_map.h
  stc/entc_udc.h
)

SET (ENTC_STC_SOURCES

  stc/entc_str.c
  stc/entc_list.c
  stc/entc_map.c
  stc/entc_udc.c
)


SET (ENTC_SYS_HEADERS

  sys/entc_types.h
  sys/entc_export.h

  sys/entc_err.h
  sys/entc_mutex.h
  sys/entc_thread.h
)

SET (ENTC_SYS_SOURCES

  sys/entc_err.c
  sys/entc_mutex.c
  sys/entc_thread.c
)

#------------------------------------------------------------------------------

source_group("System" FILES ${ENTC_SYSTEM_SOURCES} ${ENTC_SYSTEM_HEADERS})
source_group("AIO" FILES ${ENTC_AIO_SOURCES} ${ENTC_AIO_HEADERS})
source_group("Types" FILES ${ENTC_TYPES_SOURCES} ${ENTC_TYPES_HEADERS})
source_group("Tools" FILES ${ENTC_TOOLS_SOURCES} ${ENTC_TOOLS_HEADERS})
source_group("Tests" FILES ${ENTC_TESTS_SOURCES} ${ENTC_TESTS_HEADERS})

source_group("STC" FILES ${ENTC_STC_SOURCES} ${ENTC_STC_HEADERS})
source_group("SYS" FILES ${ENTC_SYS_SOURCES} ${ENTC_SYS_HEADERS})

#------------------------------------------------------------------------------

INCLUDE_DIRECTORIES("." ${CRYPT_INCLUDES} ${PTHREAD_INCLUDES} ${DL_INCLUDES})

ADD_LIBRARY( entc SHARED ${ENTC_SYSTEM_SOURCES} ${ENTC_SYSTEM_HEADERS} ${ENTC_TYPES_HEADERS} ${ENTC_AIO_SOURCES} ${ENTC_AIO_HEADERS} ${ENTC_TYPES_SOURCES} ${ENTC_TOOLS_SOURCES} ${ENTC_TOOLS_HEADERS} ${ENTC_TESTS_SOURCES} ${ENTC_TESTS_HEADERS} ${ENTC_STC_SOURCES} ${ENTC_STC_HEADERS} ${ENTC_SYS_SOURCES} ${ENTC_SYS_HEADERS})

TARGET_LINK_LIBRARIES( entc ${SYSTEM_LIBRARIES} ${CRYPT_LIBRARIES} ${PTHREAD_LIBRARIES} ${DL_LIBRARIES})

set_target_properties(entc PROPERTIES VERSION 1.4.0 SOVERSION 1.4.0)
set_property(TARGET entc PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

#------------------------------------------------------------------------------

install (TARGETS entc DESTINATION lib)
install (FILES ${ENTC_SYSTEM_HEADERS} DESTINATION include/entc/system)
install (FILES ${ENTC_AIO_HEADERS} DESTINATION include/entc/aio)
install (FILES ${ENTC_TYPES_HEADERS} DESTINATION include/entc/types)
install (FILES ${ENTC_TOOLS_HEADERS} DESTINATION include/entc/tools)
install (FILES ${ENTC_TESTS_HEADERS} DESTINATION include/entc/tests)
install (FILES ${ENTC_STC_HEADERS} DESTINATION include/entc/stc)
install (FILES ${ENTC_SYS_HEADERS} DESTINATION include/entc/sys)

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

