# - Find adbl
# Find the native ADBL headers and libraries.
#
#  ADBL_INCLUDE_DIRS - where to find headers
#  ADBL_LIBRARIES    - where to find the library
#  ADBL_FOUND        - true if entc dependencies found

#=============================================================================
#
# Copyright (c) 2010-2015 "Alexander Kalkhof" [email:adbl@kalkhof.org]
#
# This file is part of the extension n' tools (entc-base) framework for C.
#
# entc-base is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# entc-base is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
#
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

SET( ADBL_VERSION 1.0.5 )

FIND_LIBRARY(ADBL_LIBRARY NAMES adbl
  HINTS
  "/usr/lib/"
  "/usr/local/lib/"
  "/opt/local/lib/"
  "C:/Program Files/DevCommon/lib/"
  VERSION_EQUAL ${ADBL_VERSION}
)
MARK_AS_ADVANCED(ADBL_LIBRARY)

# Look for the header file.
FIND_PATH(ADBL_INCLUDE_DIR NAMES adbl.h
  PATHS 
  # on Linux/Unix 
  "/usr/include/libadbl-${ADBL_VERSION}/"
  # on Mac OSX
  "/usr/local/include/libadbl-${ADBL_VERSION}/"
  # on windows
  "C:/Program Files/DevCommon/include/adbl-${ADBL_VERSION}/"
)
MARK_AS_ADVANCED(ADBL_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set ADBL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ADBL DEFAULT_MSG ADBL_LIBRARY ADBL_INCLUDE_DIR)

IF(ADBL_FOUND)
  SET(ADBL_LIBRARIES ${ADBL_LIBRARY})
  SET(ADBL_INCLUDE_DIRS ${ADBL_INCLUDE_DIR})
ENDIF(ADBL_FOUND)
