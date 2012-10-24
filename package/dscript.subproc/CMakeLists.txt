cmake_minimum_required(VERSION 2.6)
set(PACKAGE_SOURCE_CODE subproc_glue.c)
if(NOT MINGW AND NOT MSVC)
	#add_custom_command(OUTPUT
	#	${CMAKE_CURRENT_BINARY_DIR}/files/helloworld.txt
	#	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	#	${CMAKE_CURRENT_SOURCE_DIR}/test/files/helloworld.txt
	#	${CMAKE_CURRENT_BINARY_DIR}/files/helloworld.txt)
	#add_custom_target(copy_subproc_test_files ALL DEPENDS
	#	${CMAKE_CURRENT_BINARY_DIR}/files/helloworld.txt)
	add_konoha_package(dscript.subproc)
endif()
