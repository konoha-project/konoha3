cmake_minimum_required(VERSION 2.6)

find_program(PYTHON_CONFIG NAMES python-config)
if(PYTHON_CONFIG)
	execute_process(
		COMMAND ${PYTHON_CONFIG} --include
		OUTPUT_VARIABLE PYCNF_INCLUDE_RESULT
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	string(REGEX MATCH "/[^ ]+/python[2-3].[0-9]" PYTHON_INCLUDE_PATH ${PYCNF_INCLUDE_RESULT})
	string(REGEX REPLACE "/[^ ]+/(python[2-3].[0-9])" "\\1" python_ver "${PYTHON_INCLUDE_PATH}")
	include_directories(${PYTHON_INCLUDE_PATH})
	set(PACKAGE_EXTRA_LIBRARY ${python_ver})
	set(PACKAGE_SOURCE_CODE python_glue.c)
	add_konoha_package(Lib.Python)
else(PYTHON_CONFIG)
	show_package_warning(python)
endif(PYTHON_CONFIG)

