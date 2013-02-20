# Find Event
# Once done this will define
#
#  EVENT_INCLUDE_DIR - where to find event.h
#  EVENT_LIBRARIES   - where to find libevent.{dylib|so|dll}
#  EVENT_FOUND       - True if event found.
#
if(EVENT_INCLUDE_DIR)
# Already in cache, be silent
	set(EVENT_FIND_QUIETLY TRUE)
endif(EVENT_INCLUDE_DIR)

if(APPLE)
	# support MacPorts
	FIND_PATH(EVENT_INCLUDE_DIR event2/event.h
		PATHS
		/opt/local/include/
		NO_CMAKE_SYSTEM_PATH
	)
	FIND_LIBRARY(EVENT_LIBRARIES NAMES event libevent
		PATHS
		/opt/local/lib/
		NO_CMAKE_SYSTEM_PATH
	)
endif(APPLE)

FIND_PATH(EVENT_INCLUDE_DIR event2/event.h PATHS /opt/local/include/)
string(REGEX REPLACE "(.*)/include/?" "\\1" EVENT_INCLUDE_BASE_DIR "${EVENT_INCLUDE_DIR}")

FIND_LIBRARY(EVENT_LIBRARIES NAMES event libevent
	HINTS "${EVENT_INCLUDE_BASE_DIR}/lib" PATHS /opt/local/lib)

if(EVENT_INCLUDE_DIR AND EVENT_LIBRARIES)
	SET(EVENT_FOUND TRUE)
endif(EVENT_INCLUDE_DIR AND EVENT_LIBRARIES)

if(EVENT_FOUND)
	if(NOT EVENT_FIND_QUIETLY)
		MESSAGE(STATUS "Found libevent: ${EVENT_LIBRARIES}")
	endif(NOT EVENT_FIND_QUIETLY)
else(EVENT_FOUND)
	if(EVENT_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find libevent")
	endif(EVENT_FIND_REQUIRED)
endif(EVENT_FOUND)

MARK_AS_ADVANCED(
	EVENT_INCLUDE_DIR
	EVENT_LIBRARIES
)
