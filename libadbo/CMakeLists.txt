cmake_minimum_required(VERSION 2.4)

PROJECT(adbo)

GET_TARGET_PROPERTY(ENTC_LIBRARY entc "LOCATION")
IF(ENTC_LIBRARY-NOTFOUND)
	MESSAGE(STATUS "**** entc external ****")
ELSE()
	MESSAGE(STATUS "**** entc internal ****")
	SET(ENTC_LIBRARIES ${ENTC_LIBRARY})
	SET(ENTC_INCLUDE_DIRS ../entc)
endif()

find_package(Entc)

GET_TARGET_PROPERTY(ADBL_LIBRARY adbl "LOCATION")
IF(ADBL_LIBRARY-NOTFOUND)
	MESSAGE(STATUS "**** adbl external ****")
ELSE()
	MESSAGE(STATUS "**** adbl internal ****")
	SET(ADBL_LIBRARIES ${ADBL_LIBRARY})
	SET(ADBL_INCLUDE_DIRS ../libadbl)
endif()

find_package(Adbl)

add_subdirectory(core)
add_subdirectory(tests)
