cmake_minimum_required(VERSION 2.6)
set(PACKAGE_SOURCE_CODE python_glue.c)
set(PACKAGE_SCRIPT_CODE python_glue.k)

find_program(PYTHON_CONFIG NAMES python-config)
if (PYTHON_CONFIG)
execute_process(
		COMMAND ${PYTHON_CONFIG} --include
		OUTPUT_VARIABLE PYCNF_INCLUDE_RESULT
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

string(REGEX MATCH "/[^ ]+/python[2-3].[0-9]" PYTHON_INCLUDE_PATH ${PYCNF_INCLUDE_RESULT})
string(REGEX REPLACE "/[^ ]+/(python[2-3].[0-9])" "\\1" python_ver "${PYTHON_INCLUDE_PATH}")

find_library(PYTHON_LIBRARY NAMES ${python_ver})
set(PACKAGE_EXTRA_LIBRARY ${PYTHON_LIBRARY})
include_directories(${PYTHON_INCLUDE_PATH})
add_konoha_package(python)
endif (PYTHON_CONFIG)
