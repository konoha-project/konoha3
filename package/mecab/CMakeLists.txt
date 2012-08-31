cmake_minimum_required(VERSION 2.6)
find_library(HAVE_LIBMECAB NAMES mecab)

if(HAVE_LIBMECAB)
	set(USING_ORIGINAL_MECAB_LIBS 1)
	if(APPLE)
		string(COMPARE EQUAL "/usr/lib/libmecab.dylib" ${HAVE_LIBMECAB} USE_ORIGINAL_LIB)
		if(${USE_ORIGINAL_LIB})
		set(${USING_ORIGINAL_MECAB_LIBS} 0)
		endif(${USE_ORIGINAL_LIB})
	endif(APPLE)
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/mecab_config.h.in"
		"${CMAKE_CURRENT_BINARY_DIR}/mecab_config.h"
	)
	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBMECAB})
	set(PACKAGE_EXTRA_LIBRARY ${ExtraLibs})
	set(PACKAGE_SOURCE_CODE mecab_glue.c)
	set(PACKAGE_SCRIPT_CODE mecab_glue.k)
	include_directories("${CMAKE_CURRENT_BINARY_DIR}")
	add_konoha_package(mecab)
endif(HAVE_LIBMECAB)
