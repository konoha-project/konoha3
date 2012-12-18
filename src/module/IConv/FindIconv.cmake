# Find Iconv
# Once done this will define
#
#  ICONV_INCLUDE_DIR - where to find iconv.h
#  ICONV_LIBRARIES   - where to find libiconv.{dylib|so|dll}
#  ICONV_FOUND       - True if Iconv found.
#
if(ICONV_INCLUDE_DIR)
# Already in cache, be silent
	set(ICONV_FIND_QUIETLY TRUE)
endif(ICONV_INCLUDE_DIR)

if(APPLE)
	# support MacPorts
	FIND_PATH(ICONV_INCLUDE_DIR iconv.h
		PATHS
		/opt/local/include/
		NO_CMAKE_SYSTEM_PATH
	)
	FIND_LIBRARY(ICONV_LIBRARIES NAMES iconv libiconv c
		PATHS
		/opt/local/lib/
		NO_CMAKE_SYSTEM_PATH
	)
endif(APPLE)

FIND_PATH(ICONV_INCLUDE_DIR iconv.h PATHS /opt/local/include)
string(REGEX REPLACE "(.*)/include/?" "\\1" ICONV_INCLUDE_BASE_DIR "${ICONV_INCLUDE_DIR}")

FIND_LIBRARY(ICONV_LIBRARIES NAMES iconv libiconv c
	HINTS "${ICONV_INCLUDE_BASE_DIR}/lib" PATHS /opt/local/lib)

if(ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
	SET(ICONV_FOUND TRUE)
endif(ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)

if(ICONV_FOUND)
	if(NOT ICONV_FIND_QUIETLY)
		MESSAGE(STATUS "Found Iconv: ${ICONV_LIBRARIES}")
	endif(NOT ICONV_FIND_QUIETLY)
else(ICONV_FOUND)
	if(Iconv_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find Iconv")
	endif(Iconv_FIND_REQUIRED)
endif(ICONV_FOUND)

MARK_AS_ADVANCED(
	ICONV_INCLUDE_DIR
	ICONV_LIBRARIES
)
