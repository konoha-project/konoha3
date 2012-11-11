include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckLibraryExists)

cmake_minimum_required(VERSION 2.6)

project(minikonoha)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "2")
set(VERSION_PATCH "0")

SET(CFLAGS "-Wfloat-equal -Wsign-compare -Wformat")
SET(CFLAGS "${CFLAGS} -Wall -Wcomment -Wunused-macros")
SET(CMAKE_C_FLAGS   "${CFLAGS} -Wno-long-long")
SET(CMAKE_CXX_FLAGS "${CFLAGS}")
if(MSVC)
	SET(CMAKE_C_FLAGS_RELEASE      "/D NDEBUG /W3 /Wall /TP /MT /Ox /Ob2 /GL")
	SET(CMAKE_C_FLAGS_DEBUG        "/D_DEBUG /W3 /Wall /TP /MTd /Zi /Ob0 /Od /RTC1")
	SET(CMAKE_C_FLAGS_COMPRESSED   "/D NDEBUG /W3 /Wall /TP /MT /Ox /GL")
else(MSVC)
	SET(CMAKE_C_FLAGS_RELEASE      "${CMAKE_C_FLAGS} -O3 -g")
	SET(CMAKE_C_FLAGS_DEBUG        "${CMAKE_C_FLAGS} -O0 -g3")
	SET(CMAKE_C_FLAGS_COMPRESSED   "${CMAKE_C_FLAGS} -Os -g0")
endif(MSVC)
SET(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS} -O3 -g")
SET(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS} -O0 -g3")
SET(CMAKE_CXX_FLAGS_COMPRESSED "${CMAKE_CXX_FLAGS} -Os -g0")

## Source Code settings
set(RUNTIME
	src/konoha/minikonoha.c
)

set(PARSER
	src/sugar/sugar.c
)

set(KONOHA_VM_MODULE "" CACHE STRING "konoha vm module")
if(KONOHA_VM_MODULE)
	set(VM module/${KONOHA_VM_MODULE}vm/${KONOHA_VM_MODULE}compiler.c)
else(KONOHA_VM_MODULE)
	set(VM src/vm/compiler.c)
endif(KONOHA_VM_MODULE)

## Packages
set(PACKAGES
## YOU MUST add packages in an alphabetical order 
	cstyle
	cstyle.file
	cstyle.float
	cstyle.global
	cstyle.libc
	cstyle.math
## YOU MUST add packages in an alphabetical order
	konoha
	konoha.array
	konoha.bytes
	konoha.class
	konoha.complex
	konoha.console
	konoha.const
	konoha.date
	konoha.defined
	konoha.dynamic
	konoha.eval
	konoha.eventlistener
	konoha.field
	konoha.file
	konoha.float
	konoha.foreach
	konoha.global
	konoha.i
	konoha.import
	konoha.iterator
	konoha.json
	konoha.math
	konoha.map
#	konoha.mpi
	konoha.namespace
	konoha.new
	konoha.object
	konoha.regexp
	konoha.string
##konoha.sql
	konoha.untyped
	konoha.var
	minikonoha
	sugar
	posix.fd
	posix.path
	posix.process
	posix.socket
	posix.stat
	posix.thread
	dscript.dollar
	dscript.subproc
	dscript.shell
	curl
	llvm
	libvirt
##	ijit
	jansson
	mecab
	python
#	apache   ## removed Module bugs
#	openssl
	gmp
)

set(PACKAGE_NAME    ${PROJECT_NAME})
set(PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(PACKAGE_STRING  ${CMAKE_PROJECT_NAME}-${PACKAGE_VERSION})
set(SO_VERSION      ${VERSION_MAJOR}.${VERSION_MINOR})
set(PACKAGE_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(K_CONFIG_ARCH ${CMAKE_SYSTEM})
set(K_CONFIG_OS   ${CMAKE_SYSTEM_NAME})
set(K_CONFIG_CPU  ${CMAKE_SYSTEM_PROCESSOR})
set(K_PREFIX ${CMAKE_INSTALL_PREFIX})

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
if(${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} /usr/pkg/include)
endif(${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
include_directories(${INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

string(TOUPPER ${CMAKE_BUILD_TYPE} uppercase_CMAKE_BUILD_TYPE)

if(${CMAKE_BUILD_TYPE} STREQUAL "Compressed")
	add_definitions(-DNDEBUG)
	add_definitions(-DUSE_SMALLBUILD)
endif()

if(UNIX OR CYGWIN)
	set(K_OSDLLEXT ".so")
endif()

if(APPLE)
	set(K_OSDLLEXT ".dylib")
endif(APPLE)

if(MSVC OR MINGW)
	set(K_OSDLLEXT ".dll")
endif()

find_library(HAVE_LIBDL    NAMES dl)
find_library(HAVE_LIBICONV NAMES iconv)
find_library(HAVE_PTHREAD  NAMES pthread)
find_library(HAVE_LIBDB    NAMES db)

check_type_size("void *" SIZEOF_VOIDP)
check_type_size(long     SIZEOF_LONG)
check_type_size(int      SIZEOF_INT)
check_type_size(float    SIZEOF_FLOAT)
check_type_size(double   SIZEOF_DOUBLE)

SET(CMAKE_EXTRA_INCLUDE_FILES pthread.h)
CHECK_TYPE_SIZE(pthread_mutex_t SIZEOF_PTHREAD_MUTEX_T)
SET(CMAKE_EXTRA_INCLUDE_FILES)

check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(iconv.h HAVE_ICONV_H)
check_include_files(syslog.h HAVE_SYSLOG_H)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(db.h      HAVE_DB_H)

set(_HEADERS stdio.h stdlib.h string.h ctype.h)
if(UNIX)
	set(_HEADERS ${_HEADERS} sys/mman.h unistd.h)
endif(UNIX)

if(APPLE)
	set(_HEADERS ${_HEADERS} mach-o/dyld.h)
endif(APPLE)

check_symbol_exists(posix_memalign "${_HEADERS}" HAVE_POSIX_MEMALIGN)
check_symbol_exists(memalign       "${_HEADERS}" HAVE_MEMALIGN)
check_symbol_exists(__builtin_ctzl "${_HEADERS}" HAVE_BUILTIN_CTZL)
check_symbol_exists(bzero "${_HEADERS}" HAVE_BZERO)

FIND_FILE(HAS_DOTGIT .git PATHS "${CMAKE_CURRENT_SOURCE_DIR}")
if("${HAS_DOTGIT}" MATCHES ".git")
	find_package(Git)
	if(GIT_EXECUTABLE)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-parse -q --short=8 HEAD
			OUTPUT_VARIABLE GIT_SHA1
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	set(K_REVISION "${GIT_SHA1}")
	set(K_REVISION_PREFIX "git:")
	endif(GIT_EXECUTABLE)
else()
	set(K_REVISION "${PACKAGE_VERSION}")
	set(K_REVISION_PREFIX "release:")
endif()

add_definitions(-DHAVE_CONFIG_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
		${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(ExtraLibs)
set(ExtraLibs ${ExtraLibs} ${HAVE_PTHREAD})
if(HAVE_LIBDL)
	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBDL})
endif(HAVE_LIBDL)

if(HAVE_LIBICONV)
	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBICONV})
endif(HAVE_LIBICONV)

if(HAVE_LIBDB)
	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBDB})
endif(HAVE_LIBDB)

macro(POP _STR _DELIM _RESULT)
	string(REPLACE ${_DELIM} ";" _tmp ${_STR})
	list(REVERSE _tmp)
	list(GET _tmp 0 ${_RESULT})
endmacro()

# for jenkins coverage test
option(TEST_COVERAGE "Coverage Test" OFF)
if(TEST_COVERAGE)
	include(CTest)
	message(STATUS "TEST_COVERAGE = ${TEST_COVERAGE}")
	add_definitions(-fprofile-arcs -ftest-coverage)
	set(ExtraLibs ${ExtraLibs} -fprofile-arcs -ftest-coverage)
endif(TEST_COVERAGE)

# for .k script test
enable_testing()
set(TEST_BIN ${CMAKE_CURRENT_BINARY_DIR}/minikonoha)
set(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PARENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

macro(add_test_with_property test_name)
	add_test(${test_name} ${TEST_BIN} ${ARGN})
	set_tests_properties(${test_name} PROPERTIES TIMEOUT 5)
	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT
		"KONOHA_HOME=${PARENT_BINARY_DIR}")
endmacro(add_test_with_property test_name)

option(TEST_STRESS "Use Stress Test" OFF)
macro(add_konoha_test test_name test_file)
	if(test_name MATCHES "Stress")
		if(TEST_STRESS)
			add_test_with_property(${test_name} -MOutputTest ${test_file})
			set_tests_properties(${test_name} PROPERTIES TIMEOUT 10)
		endif(TEST_STRESS)
	else(test_name MATCHES "Stress")
		if(NOT test_name MATCHES "^Shell")
			add_test_with_property(${test_name} -MOutputTest ${test_file})
		endif(NOT test_name MATCHES "^Shell")
	endif(test_name MATCHES "Stress")
endmacro(add_konoha_test test_name test_file)

file(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/test/*.k)
foreach(f ${test_files})
	string(REPLACE // / f ${f})
	string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/test/ "" test_name ${f})
	add_konoha_test(${test_name} ${f})
endforeach(f ${test_files})

macro(SET_CXX_PROPERTY_IF_MSVC TARGET)
	if(MSVC)
		foreach(arg ${ARGN})
			set_source_files_properties(${arg} PROPERTIES LANGUAGE CXX )
		endforeach(arg ${ARGN})
		set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX )
	endif(MSVC)
endmacro(SET_CXX_PROPERTY_IF_MSVC TARGET)

add_library(minikonoha SHARED ${RUNTIME} ${PARSER} ${VM})
SET_CXX_PROPERTY_IF_MSVC(minikonoha ${RUNTIME} ${PARSER} ${VM})
target_link_libraries(minikonoha ${ExtraLibs})
set(ExtraLibs ${ExtraLibs} minikonoha)

add_executable(minikonoha_bin platform/shell/command.c)
SET_CXX_PROPERTY_IF_MSVC(minikonoha_bin platform/shell/command.c)
target_link_libraries(minikonoha_bin ${ExtraLibs})
set_target_properties(minikonoha_bin PROPERTIES OUTPUT_NAME minikonoha)

# package

macro(PKGNAME _STR _RESULT)
	set(_tmp)
	POP(${_STR} "/" _tmp)
	POP(${_tmp} "." ${_RESULT})
endmacro()

macro(ENABLE_COVERAGE_INFO_IF_ENABLE VARNAME)
	if(TEST_COVERAGE)
		add_definitions(-fprofile-arcs -ftest-coverage)
		set(${VARNAME} ${${VARNAME}} -fprofile-arcs -ftest-coverage)
	endif(TEST_COVERAGE)
endmacro(ENABLE_COVERAGE_INFO_IF_ENABLE VARNAME)

macro(show_package_warning)
	if(${ARGC} EQUAL 2)
		set(WARNINGS ${WARNINGS} "Disabled ${ARGV0} package because ${ARGV1} is not found." PARENT_SCOPE)
	else(${ARGC} EQUAL 2)
		set(WARNINGS ${WARNINGS} "Disabled ${ARGV0} package because dependency libraries are not found." PARENT_SCOPE)
	endif(${ARGC} EQUAL 2)
endmacro(show_package_warning)

macro(find_package_extra_library)
	foreach(_PKGLIB ${ARGN})
		find_library(HAVE_LIB${_PKGLIB} NAMES ${_PKGLIB})
		if(HAVE_LIB${_PKGLIB})
			set(PACKAGE_EXTRA_LIBRARY ${PACKAGE_EXTRA_LIBRARY} ${HAVE_LIB${_PKGLIB}})
		else(HAVE_LIB${_PKGLIB})
			set(__NOT_FOUND ${__NOT_FOUND} ${_PKGLIB})
			set(__NOT_BUILD true)
		endif(HAVE_LIB${_PKGLIB})
	endforeach(_PKGLIB ${ARGN})
endmacro(find_package_extra_library)

macro(add_konoha_package _PKGNAME)
	if(NOT __NOT_BUILD)
		project(${_PKGNAME})
		message(STATUS "Compiling ${_PKGNAME} package")
		ENABLE_COVERAGE_INFO_IF_ENABLE(PACKAGE_EXTRA_LIBRARY)
		file(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/test/*.k)
		foreach(f ${test_files})
			string(REPLACE // / f ${f})
			string(REPLACE ${PARENT_SOURCE_DIR}/package/ "" test_name ${f})
			string(REPLACE /test "" test_name ${test_name})
			add_konoha_test(${test_name} ${f})
			if(PACKAGE_ENV)
				set_tests_properties(${test_name} PROPERTIES ENVIRONMENT ${PACKAGE_ENV})
			endif(PACKAGE_ENV)
		endforeach(f ${test_files})
		set(CMAKE_BUILD_TYPE ${KONOHA_BUILD_TYPE})
		PKGNAME(${_PKGNAME} _PACKAGE_NAME)
		set(LIB  "${_PACKAGE_NAME}_glue")
		set(_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
		set(_PACKAGE_STRING  ${CMAKE_PROJECT_NAME}-${_PACKAGE_VERSION})
		set(_KONOHA_PACKAGE_DIR lib/minikonoha/${SO_VERSION}/package/${KONOHA_VERSION}/${PROJECT_NAME})
		set(CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}
			"${CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}} ${PACKAGE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}
			"${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}} ${PACKAGE_CXX_FLAGS}")
		set(_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
				${CMAKE_CURRENT_SOURCE_DIR}/include  ${CMAKE_CURRENT_BINARY_DIR}
				${KONOHA_INCLUDE_DIRS}
				${_PACKAGE_INCLUDE_DIR})
		include_directories(${_INCLUDE_DIRS})
		list(LENGTH PACKAGE_SOURCE_CODE LEN)
		if(${LEN} GREATER 0)
			add_library(${LIB} SHARED ${PACKAGE_SOURCE_CODE})
			SET_CXX_PROPERTY_IF_MSVC(${LIB} ${PACKAGE_SOURCE_CODE})
			set_target_properties(${LIB} PROPERTIES PREFIX "")
			if(PACKAGE_EXTRA_LIBRARY)
				target_link_libraries(${LIB} ${PACKAGE_EXTRA_LIBRARY})
			endif(PACKAGE_EXTRA_LIBRARY)
			install(TARGETS ${LIB} DESTINATION ${_KONOHA_PACKAGE_DIR})
		endif(${LEN} GREATER 0)
		if(PACKAGE_SCRIPT_CODE)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE_SCRIPT_CODE}
				DESTINATION ${_KONOHA_PACKAGE_DIR})
			add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_SCRIPT_CODE}
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE_SCRIPT_CODE}
				${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_SCRIPT_CODE})
			add_custom_target(copy_${PACKAGE_SCRIPT_CODE} ALL DEPENDS
				${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_SCRIPT_CODE})
		endif(PACKAGE_SCRIPT_CODE)
		# for copy test files
		if(PACKAGE_TEST_FILE)
			foreach(testfile ${PACKAGE_TEST_FILE})
				add_custom_command(OUTPUT
					${CMAKE_CURRENT_BINARY_DIR}/${testfile}
					COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${CMAKE_CURRENT_SOURCE_DIR}/test/${testfile}
					${CMAKE_CURRENT_BINARY_DIR}/${testfile})
			endforeach(testfile ${PACKAGE_TEST_FILE})
			add_custom_target(copy_${_PKGNAME}_testfile ALL DEPENDS
				${PACKAGE_TEST_FILE})
		endif(PACKAGE_TEST_FILE)
	else(NOT __NOT_BUILD)
		foreach(_PKGLIB ${__NOT_FOUND})
			show_package_warning(${_PKGNAME} ${_PKGLIB})
		endforeach(_PKGLIB ${__NOT_FOUND})
	endif(NOT __NOT_BUILD)
endmacro(add_konoha_package)

add_subdirectory(module)

foreach(dir ${PACKAGES})
	set(dir "${CMAKE_CURRENT_SOURCE_DIR}/package/${dir}")
	if(IS_DIRECTORY ${dir} AND EXISTS ${dir}/CMakeLists.txt)
		add_subdirectory(${dir})
	endif()
endforeach()

macro(add_clib_test test_name)
	add_executable(${test_name} ${ARGN})
	SET_CXX_PROPERTY_IF_MSVC(${test_name} ${ARGN})
	target_link_libraries(${test_name} ${ExtraLibs})
	add_test(Clib/Release/${test_name} ${test_name})
	set_tests_properties(Clib/Release/${test_name} PROPERTIES TIMEOUT 5)
endmacro(add_clib_test sources)

add_clib_test(test_sizeof  ./test/Clib/test_sizeof.c)
add_clib_test(test_Kmalloc ./test/Clib/test_Kmalloc.c)
add_clib_test(test_Karray  ./test/Clib/test_Karray.c)
add_clib_test(test_gc      ./test/Clib/test_gc.c)
add_clib_test(test_Kwb     ./test/Clib/test_Kwb.c)
add_clib_test(test_map     ./test/Clib/test_map.c)
add_clib_test(test_string  ./test/Clib/test_string.c)
add_clib_test(test_cpp     ./test/Clib/test_cpp.cpp)
add_clib_test(test_proto   ./test/Clib/test_proto.c)

macro(add_builtin_test test_name)
	add_test_with_property(Builtin/${test_name} ${ARGN})
endmacro(add_builtin_test test_name)

add_builtin_test(Release/Logger  --builtin-test=test_logger)
add_builtin_test(Release/Kwb     --builtin-test=test_kwb)
add_builtin_test(Release/Kvproto --builtin-test=test_kvproto)

macro(set_package_path test_name)
set_tests_properties(${test_name} PROPERTIES ENVIRONMENT
	"KONOHA_HOME=${PARENT_BINARY_DIR}")
endmacro(set_package_path test_name)

add_test(Shell/Release/Oneline
	${CMAKE_CURRENT_SOURCE_DIR}/test/test-oneline.sh ${TEST_BIN}
	"System.p(\"hi\");")
set_package_path(Shell/Release/Oneline)

add_test(Shell/Release/Multiline
	${CMAKE_CURRENT_SOURCE_DIR}/test/test-multiline.sh ${TEST_BIN}
	${CMAKE_CURRENT_SOURCE_DIR}/test/Release/Fibonacci.k)
set_package_path(Shell/Release/Multiline)

file(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/test/Shell/*.sh)
foreach(f ${test_files})
	string(REPLACE // / f ${f})
	string(REPLACE .sh .k TEST_SCRIPT ${f})
	string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/test/ "" test_name ${TEST_SCRIPT})
	if(EXISTS ${TEST_SCRIPT})
		add_test(${test_name} sh +x ${f} ${TEST_BIN} ${TEST_SCRIPT})
	else(EXISTS ${TEST_SCRIPT})
		add_test(${test_name} sh +x ${f} ${TEST_BIN})
	endif(EXISTS ${TEST_SCRIPT})
	set_tests_properties(${test_name} PROPERTIES TIMEOUT 5)
	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT
		"KONOHA_HOME=${PARENT_BINARY_DIR}")
endforeach(f ${test_files})

option(USE_TRACEVM "Use tracevm" ON)
if(USE_TRACEVM)
	add_definitions(-DK_USE_TRACEVM)
endif(USE_TRACEVM)

add_subdirectory(platform/dse)
add_subdirectory(platform/apache)
add_subdirectory(platform/logger-shell)

include(${CMAKE_CURRENT_SOURCE_DIR}/cpack.cmake)

# for konoha command
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/platform/shell/konoha.sh.cmake
		${CMAKE_CURRENT_BINARY_DIR}/konoha.sh)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/konoha.sh
	DESTINATION bin
	RENAME konoha
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(TARGETS minikonoha minikonoha_bin
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib${LIB_SUFFIX}
		)

install(DIRECTORY include/ DESTINATION include 
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN ".git" EXCLUDE
		)

MESSAGE(STATUS)
MESSAGE(STATUS "---------------------------------------------------------------------------" )
MESSAGE(STATUS "CMAKE_BUILD_TYPE = ${uppercase_CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
MESSAGE(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS "CMAKE_C_FLAGS   = ${CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}}")
MESSAGE(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE(STATUS "---------------------------------------------------------------------------" )
MESSAGE(STATUS)
IF(WARNINGS)
	IF(TEST_COVERAGE)
		SET(MSG_STATUS SEND_ERROR)
	ELSE(TEST_COVERAGE)
		SET(MSG_STATUS STATUS)
	ENDIF(TEST_COVERAGE)
	FOREACH(WARN ${WARNINGS})
		MESSAGE(${MSG_STATUS} "[WARNING] " ${WARN})
	ENDFOREACH(WARN ${WARNINGS})
	MESSAGE(STATUS)
ENDIF(WARNINGS)
