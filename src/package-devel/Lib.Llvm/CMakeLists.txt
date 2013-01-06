cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE ${KONOHA_BUILD_TYPE})

project(llvm)
set(PACKAGE_SOURCE_CODE llvm.cpp)
set(PACKAGE_SCRIPT_CODE llvm_glue.k)

set(LLVM_INSTALL_DIR "" CACHE STRING "Install dir which llvm installed")
find_program(LLVM_CONFIG
   NAMES llvm-config-3.1
         llvm-config
         llvm-config-3.0
         llvm-config-2.9
         llvm-config-2.8
   PATHS ${LLVM_INSTALL_DIR}
         /usr/local/bin/
         /usr/bin/
   DOC "llvm-config")
if(LLVM_CONFIG)
execute_process(
		COMMAND ${LLVM_CONFIG} --version
		OUTPUT_VARIABLE LLVM_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

execute_process(
		COMMAND ${LLVM_CONFIG} --cppflags
		OUTPUT_VARIABLE LLVM_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
execute_process(
		COMMAND ${LLVM_CONFIG} --includedir
		OUTPUT_VARIABLE LLVM_INCLUDEDIR
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
execute_process(
		COMMAND ${LLVM_CONFIG} --cxxflags
		OUTPUT_VARIABLE LLVM_CXXFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
execute_process(
		COMMAND ${LLVM_CONFIG} --ldflags
		OUTPUT_VARIABLE LLVM_LDFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
execute_process(
	COMMAND ${LLVM_CONFIG} --libs core jit native linker ipo engine interpreter
	OUTPUT_VARIABLE LLVM_JIT_LIBS
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)
execute_process(
		COMMAND ${LLVM_CONFIG} --libdir
		OUTPUT_VARIABLE LLVM_LIBDIR
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

set(ORIG_CXX_FLAGS ${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}})
set(PACKAGE_CXX_FLAGS "${LLVM_CXXFLAGS} -Wno-variadic-macros ${ORIG_CXX_FLAGS} -UNDEBUG")
string(REPLACE "-l" "" tmp ${LLVM_JIT_LIBS})
string(REPLACE " " ";" LLVMLIBS ${tmp})

string(REPLACE " " ";" tmp ${LLVM_LDFLAGS})
foreach(lib ${tmp})
	string(REGEX MATCH "^-l" res "${lib}")
	if("${res}" STREQUAL "-l")
		string(REPLACE "-l" "" lib ${lib})
		set(LLVMLIBS ${LLVMLIBS} ${lib})
	endif()
endforeach(lib ${tmp})

link_directories(${LINK_DIRECTORIES} ${LLVM_LIBDIR})

set(K_LLVM_VERSION ${LLVM_VERSION})
string(REPLACE "." "0" K_LLVM_VERSION ${K_LLVM_VERSION})
string(REPLACE "svn" "" K_LLVM_VERSION ${K_LLVM_VERSION})
add_definitions(-DLLVM_VERSION=${K_LLVM_VERSION})
include_directories(${LLVM_INCLUDEDIR})

#message(STATUS "${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}}")
if(${LLVM_VERSION} VERSION_LESS "2.8")
message(FATAL ERROR
        "llvm package not support old version of llvm (<=llvm-2.8)")
endif(${LLVM_VERSION} VERSION_LESS "2.8")

set(PACKAGE_EXTRA_LIBRARY ${LLVMLIBS})
	add_konoha_package(Lib.Llvm)
else(LLVM_CONFIG)
show_package_warning(llvm)
endif(LLVM_CONFIG)
