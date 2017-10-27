cmake_minimum_required(VERSION 2.4)

PROJECT(entc-base)

INCLUDE_DIRECTORIES( "../" )

enable_testing()

set(CTEST_PROJECT_NAME "entc tests")

ADD_EXECUTABLE( ut_eclist types/ut_eclist.c )
TARGET_LINK_LIBRARIES( ut_eclist entc )

add_test(ut_eclist tests/Debug/ut_eclist)

ADD_EXECUTABLE( ut_lparser tools/ut_lparser.c )
TARGET_LINK_LIBRARIES( ut_lparser entc )

add_test(ut_lparser tests/Debug/ut_lparser)

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
