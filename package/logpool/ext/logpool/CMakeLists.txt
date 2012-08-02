include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckCSourceCompiles)

cmake_minimum_required(VERSION 2.6)
project(logpool)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")

set(PACKAGE_NAME    ${PROJECT_NAME})
set(PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(PACKAGE_STRING  ${CMAKE_PROJECT_NAME}-${PACKAGE_VERSION})
set(SO_VERSION   ${VERSION_MAJOR}.${VERSION_MINOR})

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

SET(CMAKE_C_FLAGS         "-Wall -Wno-variadic-macros")
SET(CMAKE_CXX_FLAGS       "-Wall -Wno-variadic-macros")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -g")
SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2 -g")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -O0 -g3")

set(LOGPOOL_SRC
	core/logpool.c
	core/crc32.c
	core/map.c
)

set(LOGPOOL_DAEMON_SRC tools/logpoold.c)
set(LOGPOOL_LOGPUT_SRC tools/logput.c)
set(LOGPOOL_POOL plugins/pool/pool_plugin.c
		plugins/pool/print.c
		plugins/pool/copy.c
		plugins/pool/statics.c
		plugins/pool/time.c
		plugins/pool/react.c
		plugins/pool/key.c
		plugins/pool/val.c
		plugins/pool/stream.c
		plugins/pool/response.c
		plugins/pool/create.c
		plugins/pool/close.c
		plugins/pool/konoha_plugin.c
)

set(IO_SRC core/io.c core/stream.c core/trace.c core/server.c core/client.c)

set(LOGPOOL_TRACE
		plugins/trace/stream.c
		plugins/trace/lpstring.c
		plugins/trace/file.c
		plugins/trace/filter.c
		plugins/trace/multiplexer.c
		plugins/trace/syslog.c
)
find_library(HAVE_LIBDL    NAMES dl)
find_library(HAVE_PTHREAD  NAMES pthread)
find_library(HAVE_LIBMEMCACHED NAMES memcached)
find_library(HAVE_LIBEVENT NAMES event)
find_library(HAVE_LIBEVENT_PTHREADS NAMES event_pthreads)
find_library(HAVE_KONOHA  NAMES minikonoha)

check_include_files(pthread.h      HAVE_PTHREAD_H)
check_include_files(event2/event.h HAVE_LIBEVENT_H)
check_include_files(libmemcached/memcached.h HAVE_MEMCACHED_H)

if(NOT HAVE_PTHREAD_H)
	message(FATAL_ERROR "pthread.h not found: ${HAVE_PTHREAD_H}")
endif(NOT HAVE_PTHREAD_H)

if(NOT HAVE_LIBEVENT_H)
	message(FATAL_ERROR "event2/event.h not found")
endif(NOT HAVE_LIBEVENT_H)

if(NOT HAVE_PTHREAD)
	message(FATAL_ERROR "libpthread not found")
endif(NOT HAVE_PTHREAD)

if(NOT HAVE_LIBEVENT)
	message(FATAL_ERROR "libevent not found")
endif(NOT HAVE_LIBEVENT)

if(NOT HAVE_LIBMEMCACHED)
	message(FATAL_ERROR "libmemcached not found")
endif(NOT HAVE_LIBMEMCACHED)

set(LOGPOOL_TRACE ${LOGPOOL_TRACE} plugins/trace/memcache.c)
set(LOGPOOL_SRC ${LOGPOOL_SRC} ${LOGPOOL_POOL} ${LOGPOOL_TRACE} ${IO_SRC})
#set(LOGPOOL_SRC ${LOGPOOL_SRC} core/llcache.cpp)


check_type_size("void *" SIZEOF_VOIDP)
check_type_size(long     SIZEOF_LONG)
check_type_size(int      SIZEOF_INT)
check_type_size(float    SIZEOF_FLOAT)
check_type_size(double   SIZEOF_DOUBLE)

set(ExtraLibs)
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBMEMCACHED})
set(ExtraLibs ${ExtraLibs} ${HAVE_PTHREAD})
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBEVENT})
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBEVENT_PTHREADS})
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBDL})
set(ExtraLibs ${ExtraLibs} ${HAVE_KONOHA})


#find_program(LLVM_CONFIG NAMES llvm-config DOC "llvm-config")
#find_program(HAVE_CLANG   NAMES clang DOC "clang")
#if(NOT LLVM_CONFIG)
#	message(FATAL_ERROR "llvm-config not found")
#endif(NOT LLVM_CONFIG)
#if(NOT HAVE_CLANG)
#	message(FATAL_ERROR "clang not found")
#endif(NOT HAVE_CLANG)
##
#execute_process(
#		COMMAND ${LLVM_CONFIG} --version
#		OUTPUT_VARIABLE LLVM_VERSION
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#
#set(K_LLVM_VERSION ${LLVM_VERSION})
#string(REPLACE "." "0" K_LLVM_VERSION ${K_LLVM_VERSION})
#string(REPLACE "svn" "" K_LLVM_VERSION ${K_LLVM_VERSION})
#add_definitions(-DLLVM_VERSION=${K_LLVM_VERSION})
#include_directories(${LLVM_INCLUDEDIR})
#
#execute_process(
#		COMMAND ${LLVM_CONFIG} --cxxflags
#		OUTPUT_VARIABLE LLVM_CXXFLAGS
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#execute_process(
#		COMMAND ${LLVM_CONFIG} --ldflags
#		OUTPUT_VARIABLE LLVM_LDFLAGS
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#execute_process(
#	COMMAND ${LLVM_CONFIG} --libs core jit native linker ipo interpreter bitwriter
#	OUTPUT_VARIABLE LLVM_JIT_LIBS
#	OUTPUT_STRIP_TRAILING_WHITESPACE
#	)
#execute_process(
#		COMMAND ${LLVM_CONFIG} --libdir
#		OUTPUT_VARIABLE LLVM_LIBDIR
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#
#set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS} ${CMAKE_CXX_FLAGS}")
#string(REPLACE " " ";" v ${LLVM_JIT_LIBS})
#link_directories(${LINK_DIRECTORIES} ${LLVM_LIBDIR})
#set(ExtraLibs ${ExtraLibs} ${v})
#
#macro(add_client_module target src)
#	string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
#	set(dirs "")
#	foreach(arg ${INCLUDE_DIRS})
#		set(dirs ${dirs} "-I${arg}")
#	endforeach()
#	set(_output ${CMAKE_CURRENT_BINARY_DIR}/${target}.bc)
#	add_custom_command(
#		OUTPUT  ${_output}
#		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
#		COMMAND ${HAVE_CLANG} ${CMAKE_CURRENT_SOURCE_DIR}/${src}
#		${dirs} -O2 -g0 -emit-llvm -c -o
#		${_output})
#	add_custom_target(create_client_module_${target} DEPENDS ${_output})
#	add_dependencies(logpool create_client_module_${target})
#	add_dependencies(create_client_module_${target} CREATE_IDL_H)
#endmacro()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
		${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-fPIC)

add_library(logpool SHARED ${LOGPOOL_SRC})
target_link_libraries(logpool ${ExtraLibs})
set_target_properties(logpool PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${SO_VERSION})

#add_subdirectory(logpool-viewer)

## IDL Generator
add_definitions(-DIO_IDL_OUTPUT="${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h")
add_executable(io_idlgen tools/idl-gen.cpp)
add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h
		DEPENDS ${PROJECT_SOURCE_DIR}/message.idl
		COMMAND io_idlgen ${PROJECT_SOURCE_DIR}/message.idl
		)
add_custom_target(CREATE_IDL_H DEPENDS
		${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h)
add_dependencies(logpool CREATE_IDL_H)

#add_client_module(mod_example1 example/mod_example1.c)
#add_client_module(mod_cpuwatch example/mod_cpuwatch.c)

## Test cases
enable_testing()
add_executable(logpoold ${LOGPOOL_DAEMON_SRC})
target_link_libraries(logpoold logpool ${ExtraLibs})

add_executable(logput ${LOGPOOL_LOGPUT_SRC})
target_link_libraries(logput logpool ${ExtraLibs})

add_executable(logtail tools/logtail.c)
target_link_libraries(logtail logpool ${ExtraLibs})

#add_executable(example1 example/example1.c)
#target_link_libraries(example1 logpool ${ExtraLibs})

add_executable(cpuemu example/cpuemu.c)
target_link_libraries(cpuemu logpool ${ExtraLibs})
add_executable(cpuwatch example/cpuwatch.c)
target_link_libraries(cpuwatch logpool ${ExtraLibs})

macro(add_clib_test test_name)
	add_executable(${test_name} ${ARGN})
	target_link_libraries(${test_name} ${TestLibs})
	add_test(logpool_${test_name} ${test_name})
endmacro(add_clib_test test_name)

set(TestLibs logpool)

add_clib_test(logpool_test_string   test/string.c)
add_clib_test(logpool_test_file     test/file.c)
add_clib_test(logpool_test_memcache test/memcache.c)
add_clib_test(logpool_test_syslog   test/syslog.c)
add_clib_test(logpool_test_largefmt test/large_fmtdata.c)
add_clib_test(logpool_test_filter   test/filter.c)
add_clib_test(logpool_test_multi    test/multiplex.c)
add_clib_test(logpool_test_cpp      test/cpp.cpp)
add_clib_test(logpool_test_trace   test/trace.c)
add_clib_test(logpool_test_trace2  test/trace2.c)
add_clib_test(logpool_test_thread  test/test_thread.c)
add_executable(logpool_test_map    test/map.c)
target_link_libraries(logpool_test_map ${TestLibs})
add_test(logpool_test_map logpool_test_map ${CMAKE_CURRENT_SOURCE_DIR}/test/map-test.txt)

#add_clib_test(test_react test/react.c)
add_clib_test(logpool_test_list  test/test_list.c)
add_clib_test(logpool_test_pool  test/pool.c)

install(TARGETS logpoold RUNTIME DESTINATION bin)
install(TARGETS logpool
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		)

install(DIRECTORY include/ DESTINATION include/logpool
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN ".git" EXCLUDE
		)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h DESTINATION include/logpool)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plugins/pool/pool_plugin.h DESTINATION include/logpool)
