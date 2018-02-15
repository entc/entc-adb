cmake_minimum_required(VERSION 2.4)

PROJECT(entc-base)

INCLUDE_DIRECTORIES( "../" )

enable_testing()

set(CTEST_PROJECT_NAME "entc tests")

#----------- system ----------------------------------------------

ADD_EXECUTABLE( ut_ecfile system/ut_ecfile.c )
TARGET_LINK_LIBRARIES( ut_ecfile entc )
add_test(ut_ecfile tests/Debug/ut_ecfile)

ADD_EXECUTABLE( ut_ecaio system/ut_ecaio.c )
TARGET_LINK_LIBRARIES( ut_ecaio entc )
add_test(ut_ecaio tests/Debug/ut_ecaio)

ADD_EXECUTABLE( ut_ecaio_notify system/ut_ecaio_notify.c )
TARGET_LINK_LIBRARIES( ut_ecaio_notify entc )
add_test(ut_ecaio_notify tests/Debug/ut_ecaio_notify)

ADD_EXECUTABLE( ut_ecaio_proc system/ut_ecaio_proc.c )
TARGET_LINK_LIBRARIES( ut_ecaio_proc entc )
add_test(ut_ecaio_proc tests/Debug/ut_ecaio_proc)

ADD_EXECUTABLE( ut_ecexec system/ut_ecexec.c )
TARGET_LINK_LIBRARIES( ut_ecexec entc )
add_test(ut_ecexec tests/Debug/ut_ecexec)

#----------- types -----------------------------------------------

ADD_EXECUTABLE( ut_ecmap types/ut_ecmap.c )
TARGET_LINK_LIBRARIES( ut_ecmap entc )
add_test(ut_ecmap tests/Debug/ut_ecmap)

ADD_EXECUTABLE( ut_eclist types/ut_eclist.c )
TARGET_LINK_LIBRARIES( ut_eclist entc )
add_test(ut_eclist tests/Debug/ut_eclist)

ADD_EXECUTABLE( ut_ecstream types/ut_ecstream.c )
TARGET_LINK_LIBRARIES( ut_ecstream entc )
add_test(ut_ecstream tests/Debug/ut_ecstream)

ADD_EXECUTABLE( ut_ecstring types/ut_ecstring.c )
TARGET_LINK_LIBRARIES( ut_ecstring entc )
add_test(ut_ecstring tests/Debug/ut_ecstring)

ADD_EXECUTABLE( ut_ecbuffer types/ut_ecbuffer.c )
TARGET_LINK_LIBRARIES( ut_ecbuffer entc )
add_test(ut_ecbuffer tests/Debug/ut_ecbuffer)

ADD_EXECUTABLE( ut_ecudc types/ut_ecudc.c )
TARGET_LINK_LIBRARIES( ut_ecudc entc )
add_test(ut_ecudc tests/Debug/ut_ecudc)

#----------- tools -----------------------------------------------

ADD_EXECUTABLE( ut_lparser tools/ut_lparser.c )
TARGET_LINK_LIBRARIES( ut_lparser entc )
add_test(ut_lparser tests/Debug/ut_lparser)

ADD_EXECUTABLE( ut_ectemplate tools/ut_ectemplate.c )
TARGET_LINK_LIBRARIES( ut_ectemplate entc )
add_test(ut_ectemplate tests/Debug/ut_ectemplate)

ADD_EXECUTABLE( ut_ecjparser tools/ut_ecjparser.c )
TARGET_LINK_LIBRARIES( ut_ecjparser entc )
add_test(ut_ecjparser tests/Debug/ut_ecjparser)

ADD_EXECUTABLE( ut_ecjson tools/ut_ecjson )
TARGET_LINK_LIBRARIES( ut_ecjson entc )
add_test(ut_ecjson tests/Debug/ut_ecjson)

#-----------------------------------------------------------------


#ADD_EXECUTABLE( unittest_exec system/unittest_exec.c )
#TARGET_LINK_LIBRARIES( unittest_exec entc )

#ADD_EXECUTABLE( unittest_dh system/unittest_dh.c )
#TARGET_LINK_LIBRARIES( unittest_dh entc )

#ADD_EXECUTABLE( unittest_asyncserv tools/unittest_asyncserv.c )
#TARGET_LINK_LIBRARIES( unittest_asyncserv entc )

#ADD_EXECUTABLE( unittest_server tools/unittest_server.c )
#TARGET_LINK_LIBRARIES( unittest_server entc )

#ADD_EXECUTABLE( unittest_messages utils/unittest_ecmessages.c )
#TARGET_LINK_LIBRARIES( unittest_messages entc ) 

#ADD_EXECUTABLE( unittest_events system/unittest_events.c )
#TARGET_LINK_LIBRARIES( unittest_events entc ) 

#ADD_EXECUTABLE( unittest_bins tools/unittest_bins.c )
#TARGET_LINK_LIBRARIES( unittest_bins entc ) 

#ADD_EXECUTABLE( unittest_mime tools/unittest_mime.c )
#TARGET_LINK_LIBRARIES( unittest_mime entc ) 

#ADD_EXECUTABLE( unittest_mimeparser tools/unittest_mimeparser.c )
#TARGET_LINK_LIBRARIES( unittest_mimeparser entc )

#ADD_EXECUTABLE( unittest_json tools/unittest_json.c )
#TARGET_LINK_LIBRARIES( unittest_json entc )

#ADD_EXECUTABLE( unittest_stream types/unittest_stream.c )
#TARGET_LINK_LIBRARIES( unittest_stream entc )
