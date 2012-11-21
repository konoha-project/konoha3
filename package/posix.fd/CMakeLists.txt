cmake_minimum_required(VERSION 2.6)
if(NOT MINGW AND NOT MSVC)
set(PACKAGE_SOURCE_CODE fd_glue.c)
include(CheckStructHasMember)
check_struct_has_member("struct stat" st_rdev sys/stat.h HAVE_STRUCT_STAT_ST_RDEV)
check_struct_has_member("struct stat" st_blocks sys/stat.h HAVE_STRUCT_STAT_ST_BLOCKS)
check_struct_has_member("struct stat" st_blksize sys/stat.h HAVE_STRUCT_STAT_ST_BLKSIZE)
check_struct_has_member("struct stat" st_flags sys/stat.h HAVE_STRUCT_STAT_ST_FLAGS)
check_struct_has_member("struct stat" st_gen sys/stat.h HAVE_STRUCT_STAT_ST_GEN)
check_struct_has_member("struct stat" st_birthtime sys/stat.h HAVE_STRUCT_STAT_ST_BIRTHTIME)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
	${CMAKE_CURRENT_BINARY_DIR}/fdconfig.h)
#add_custom_command(OUTPUT
#		${CMAKE_CURRENT_BINARY_DIR}/files/readlink.txt
#		COMMAND ${CMAKE_COMMAND} -E create_symlink
#		newfile.txt
#		${CMAKE_CURRENT_BINARY_DIR}/files/readlink.txt)
#add_custom_target(link_fd_test_file ALL DEPENDS
#		${CMAKE_CURRENT_BINARY_DIR}/files/readlink.txt)
#set(PACKAGE_TEST_FILE files/another.txt files/ftruncate.txt files/newfile.txt
#	files/openfile.txt)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_konoha_package(posix.fd)
endif()
