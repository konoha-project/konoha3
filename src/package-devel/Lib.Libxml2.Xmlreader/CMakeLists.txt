cmake_minimum_required(VERSION 2.6)

find_library(HAVE_LIBXML NAMES xml2)
find_program(XML2_CONFIG NAMES xml2-config DOC "xml2-config")

if(HAVE_LIBXML)
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBXML})
set(PACKAGE_EXTRA_LIBRARY ${ExtraLibs})
set(PACKAGE_SOURCE_CODE xml_glue.c)

if(XML2_CONFIG)
execute_process(
		COMMAND ${XML2_CONFIG} --cflags
		OUTPUT_VARIABLE XML2_INCLUDE_PATH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

string(REPLACE "-I" "" XML2_INCLUDE_PATH ${XML2_INCLUDE_PATH})
include_directories(${XML2_INCLUDE_PATH})
add_konoha_package(Lib.Libxml2.Xmlreader)
endif(XML2_CONFIG)
endif(HAVE_LIBXML)
