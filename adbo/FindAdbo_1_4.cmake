# - Find adbo
# Find the native ADBO headers and libraries.
#
#  ADBO_INCLUDE_DIRS - where to find headers
#  ADBO_LIBRARIES    - where to find the library
#  ADBO_FOUND        - true if entc dependencies found

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

SET( ADBO_VERSION 1.4.0 )

FIND_LIBRARY(ADBO_LIBRARY NAMES adbo
  HINTS
  "/opt/entc_1_4/lib/"
)
MARK_AS_ADVANCED(ADBO_LIBRARY)

# Look for the header file.
FIND_PATH(ADBO_INCLUDE_DIR NAMES adbo_types.h
  PATHS 
  "/opt/entc_1_4/include/adbo"
)
MARK_AS_ADVANCED(ADBO_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set ADBO_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ADBO DEFAULT_MSG ADBO_LIBRARY ADBO_INCLUDE_DIR)

IF(ADBO_FOUND)
  SET(ADBO_LIBRARIES ${ADBO_LIBRARY})
  SET(ADBO_INCLUDE_DIRS ${ADBO_INCLUDE_DIR})
ENDIF(ADBO_FOUND)
