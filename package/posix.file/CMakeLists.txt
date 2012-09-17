cmake_minimum_required(VERSION 2.6)
set(PACKAGE_SOURCE_CODE file_glue.c)
if(NOT MINGW AND NOT MSVC)
add_custom_command(OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/files/writetest.txt
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/test/files/writetest.txt
		${CMAKE_CURRENT_BINARY_DIR}/files/writetest.txt)
add_custom_command(OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/files/testfile.txt
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/test/files/testfile.txt
		${CMAKE_CURRENT_BINARY_DIR}/files/testfile.txt)
add_custom_command(OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/files/truncate.txt
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/test/files/truncate.txt
		${CMAKE_CURRENT_BINARY_DIR}/files/truncate.txt)
add_custom_command(OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/files/opentest.txt
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/test/files/opentest.txt
		${CMAKE_CURRENT_BINARY_DIR}/files/opentest.txt)
add_custom_target(copy_file_test_files ALL DEPENDS
		${CMAKE_CURRENT_BINARY_DIR}/files/writetest.txt
		${CMAKE_CURRENT_BINARY_DIR}/files/testfile.txt
		${CMAKE_CURRENT_BINARY_DIR}/files/truncate.txt
		${CMAKE_CURRENT_BINARY_DIR}/files/opentest.txt)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_konoha_package(posix.file)
endif()
