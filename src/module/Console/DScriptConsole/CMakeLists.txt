# DScriptConsole Module
set(MODULE_SOURCE_CODE DScriptConsole.c)

find_package(LibEvent)
if(EVENT_FOUND)
	set(MODULE_INCLUDE_DIR ${EVENT_INCLUDE_DIR})
	set(MODULE_EXTRA_LIBRARY ${EVENT_LIBRARIES})
	add_konoha_module(DScriptConsole)
endif()
