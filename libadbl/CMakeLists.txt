cmake_minimum_required(VERSION 2.4)

PROJECT(adbl)

GET_TARGET_PROPERTY(ENTC_LIBRARY entc "LOCATION")
IF(ENTC_LIBRARY-NOTFOUND)
	MESSAGE(STATUS "**** entc external ****")
	find_package(Entc)
ELSE()
	MESSAGE(STATUS "**** entc internal ****")
	SET(ENTC_LIBRARIES ${ENTC_LIBRARY})
	SET(ENTC_INCLUDE_DIRS ../entc)
endif()

add_subdirectory(core)

add_subdirectory(adbl_mysql)
add_subdirectory(adbl_sqlite)
#add_subdirectory(adbl_oracle)
