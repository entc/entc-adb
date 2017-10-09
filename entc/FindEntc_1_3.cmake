# - Find entc
# Find the native ENTC headers and libraries.
#
#  ENTC_INCLUDE_DIRS - where to find headers
#  ENTC_LIBRARIES    - where to find the library
#  ENTC_FOUND        - true if entc dependencies found

#=============================================================================
#
# Copyright (c) 2010-2015 "Alexander Kalkhof" [email:alex@kalkhof.org]
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

SET(ENTC_VERSION "1.3.0")

# Look for the header file.
FIND_LIBRARY( ENTC_LIBRARY NAMES entc
  HINTS
  "/usr/lib/"
  "/usr/local/lib/"
  "/opt/local/lib/"
  "$ENV{PROGRAMFILES}/quom/entc/${ENTC_VERSION}/lib/"
)
MARK_AS_ADVANCED(ENTC_LIBRARY)

FIND_PATH(ENTC_INCLUDE_DIR
  NAMES system/macros.h
  PATHS
  # linux
  "/usr/include/entc-${ENTC_VERSION}/"
  # macosx
  "/usr/local/include/entc-${ENTC_VERSION}/"
  # windows
  "$ENV{PROGRAMFILES}/quom/entc/${ENTC_VERSION}/include/"
)
MARK_AS_ADVANCED(ENTC_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set ENTC_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ENTC DEFAULT_MSG ENTC_LIBRARY ENTC_INCLUDE_DIR)

IF(ENTC_FOUND)
  SET(ENTC_LIBRARIES ${ENTC_LIBRARY})
  SET(ENTC_INCLUDE_DIRS ${ENTC_INCLUDE_DIR})
  SET(VERSION_VAR ${ENTC_VERSION})
ENDIF(ENTC_FOUND)
