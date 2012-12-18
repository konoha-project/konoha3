include(CheckFunctionExists)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

set(MODULE_SOURCE_CODE IConv.c)

find_package(Iconv)
if(ICONV_FOUND)
	include_directories(${ICONV_INCLUDE_DIR})
	set(MODULE_EXTRA_LIBRARY ${ICONV_LIBRARIES})
	add_konoha_module(IConv)
endif()
