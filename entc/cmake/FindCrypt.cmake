#-------------------------------------------------------------------------------
# Copyright (c) 2018-2018, Alexander Kalkhof <alex@kalkhof.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#-------------------------------------------------------------------------------

# The following variables are set when CRYPT is found:
#  CRYPT_FOUND 
#  CRYPT_INCLUDES
#  CRYPT_LIBRARIES

if (NOT CRYPT_FOUND)

	IF (WIN32)
	  # we can use windows onw crypto libs
	ELSE()

	  INCLUDE(FindPackageHandleStandardArgs)

	  ##____________________________________________________________________________
	  ## Check for the header files

	  find_path (CRYPT_INCLUDES
		NAMES openssl/aes.h
		HINTS ${CMAKE_INSTALL_PREFIX}
		PATH_SUFFIXES include
		)

	  ##____________________________________________________________________________
	  ## Check for the library

	  find_library (CRYPT_LIBRARIES
		NAMES crypto crypt
		HINTS /opt/local/ ${CMAKE_INSTALL_PREFIX} 
		PATH_SUFFIXES lib
		)

	  ##____________________________________________________________________________
	  ## Actions taken when all components have been found

	  find_package_handle_standard_args (CRYPT DEFAULT_MSG CRYPT_LIBRARIES CRYPT_INCLUDES)
		
	  if (CRYPT_FOUND)
		if (NOT CRYPT_FIND_QUIETLY)
		  message (STATUS "Found components for CRYPT")
		  message (STATUS "CRYPT_INCLUDES  = ${CRYPT_INCLUDES}")
		  message (STATUS "CRYPT_LIBRARIES = ${CRYPT_LIBRARIES}")
		endif (NOT CRYPT_FIND_QUIETLY)
	  else (CRYPT_FOUND)
		if (CRYPT_FIND_REQUIRED)
		  message (FATAL_ERROR "Could not find CRYPT!")
		endif (CRYPT_FIND_REQUIRED)
	  endif (CRYPT_FOUND)

	  ##____________________________________________________________________________
	  ## Mark advanced variables

	  mark_as_advanced (
		CRYPT_ROOT_DIR
		CRYPT_INCLUDES
		CRYPT_LIBRARIES
		)

	ENDIF(WIN32)

endif (NOT CRYPT_FOUND)
