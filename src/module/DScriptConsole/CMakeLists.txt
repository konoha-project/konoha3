# DScriptConsole Module
set(MODULE_SOURCE_CODE DScriptConsole.c)

message(STATUS "${EVENT_INCLUDE_DIR}")
find_package(LibEvent)
if(EVENT_FOUND)
	include_directories(${EVENT_INCLUDE_DIR})
	set(MODULE_EXTRA_LIBRARY ${EVENT_LIBRARIES})
	add_konoha_module(DScriptConsole)
endif()
