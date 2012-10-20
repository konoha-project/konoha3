cmake_minimum_required(VERSION 2.6)
set(PACKAGE_SOURCE_CODE path_glue.c)
if(NOT MINGW AND NOT MSVC)
set(PACKAGE_TEST_FILE files/writetest.txt files/testfile.txt files/truncate.txt
	files/opentest.txt)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_konoha_package(posix.path)
endif()
