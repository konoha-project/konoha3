include(CheckTypeSize)
include(CheckIncludeFiles)

cmake_minimum_required(VERSION 2.4)
project(kjson)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

SET(CFLAGS "-Wsign-compare -Wmissing-field-initializers -Wdisabled-optimization")
SET(CFLAGS "${CFLAGS} -Wall -Wcomment -Wformat -Wunused-macros")
SET(CMAKE_C_FLAGS         "-Wall -Wno-variadic-macros ${CFLAGS}")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -g")
SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
string(TOUPPER ${CMAKE_BUILD_TYPE} uppercase_CMAKE_BUILD_TYPE)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DNDEBUG)
endif()

set(JSON_SRC kjson.c)
add_library(kjson SHARED ${JSON_SRC})

## Test cases
file(GLOB JSON_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/test*.json)
file(GLOB JSON_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/benchmark*.json)

set(JSON_TEST_CODE_LIST)
set(JSON_BENCH_CODE_LIST)
foreach(it ${JSON_TEST})
    set(JSON_TEST_CODE_LIST ${JSON_TEST_CODE_LIST} ${it})
endforeach()

foreach(it ${JSON_BENCH})
    set(JSON_BENCH_CODE_LIST ${JSON_BENCH_CODE_LIST} ${it})
endforeach()

enable_testing()

add_executable(test_mempool test/test_mempool.c)
target_link_libraries(test_mempool kjson)
add_test(kjson_test_mempool test_mempool)

add_executable(test_kjson test/test_kjson.c)
target_link_libraries(test_kjson kjson)
add_test(kjson_test_kjson test_kjson ${JSON_TEST_CODE_LIST}
        ${JSON_BENCH_CODE_LIST})

add_executable(test_stream test/test_stream.c)
target_link_libraries(test_stream kjson)
add_test(kjson_test_stream test_stream ${JSON_TEST_CODE_LIST}
        ${JSON_BENCH_CODE_LIST})

add_executable(test_stringbuilder test/test_string_builder.c)
add_test(kjson_test_stringbuilder test_stringbuilder)

add_executable(test_stack test/test_kstack.c)
add_test(kjson_test_stack test_stack)

add_executable(test_tostring test/test_tostring.c)
target_link_libraries(test_tostring kjson)
add_test(kjson_test_tostring test_tostring)

file(GLOB JSON_FAIL_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/test_json/fail*.json)
file(GLOB JSON_PASS_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/test_json/pass*.json)
add_executable(test_file test/test_file.c)

foreach(f ${JSON_FAIL_TEST})
	add_test(${f} test_file ${f} "F")
endforeach()

foreach(f ${JSON_PASS_TEST})
	add_test(${f} test_file ${f} "T")
endforeach()

add_executable(test_kexception test/test_kexception.c)
add_test(kjson_test_kexception test_kexception)

#add_executable(test_float test/test_float.c)
#add_test(kjson_test_float test_float)

## Benchmark
find_library(HAVE_LIBJSON json)
find_library(HAVE_LIBJANSSON jansson)
find_library(HAVE_LIBMSGPACK msgpack)
find_library(HAVE_LIBYAJL yajl)
set(bench1_libs)
set(bench2_libs)

if(HAVE_LIBMSGPACK)
    add_definitions(-DUSE_MSGPACK)
    set(bench1_libs ${bench1_libs} ${HAVE_LIBMSGPACK})
endif()
if(HAVE_LIBYAJL)
    add_definitions(-DUSE_YAJL)
    set(bench1_libs ${bench1_libs} ${HAVE_LIBYAJL})
    set(bench2_libs ${bench2_libs} ${HAVE_LIBYAJL})
    endif()
if(HAVE_LIBJANSSON)
    add_definitions(-DUSE_JANSSON)
    set(bench2_libs ${bench2_libs} ${HAVE_LIBJANSSON})
    elseif(HAVE_LIBJSON)
    add_definitions(-DUSE_JSON_C)
    set(bench2_libs ${bench2_libs} ${HAVE_LIBJSON})
endif()

add_executable(benchmark1 test/benchmark1.c)
target_link_libraries(benchmark1 kjson ${bench1_libs})

add_executable(benchmark2 test/benchmark2.cpp)
target_link_libraries(benchmark2 kjson ${bench2_libs})

add_executable(benchmark3 test/benchmark3.c)
target_link_libraries(benchmark3 kjson)

add_executable(benchmark4 test/benchmark4.c)
target_link_libraries(benchmark4 kjson)

add_executable(benchmark5 test/benchmark5.c)

add_executable(parseString test/parseString.c)
add_test(kjson_parseString parseString)
target_link_libraries(parseString kjson)

add_executable(check_cpp test/check_cpp_build.cpp)
add_test(kjson_check_cpp check_cpp)

add_executable(test_whitespace test/test_whitespace.c)
add_test(kjson_test_whitespace test_whitespace)
target_link_libraries(test_whitespace kjson)

install(TARGETS kjson LIBRARY DESTINATION lib)

file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
#message(STATUS ${headers})
foreach(it ${headers})
	install(FILES ${it} DESTINATION include/kjson)
endforeach(it ${headers})

MESSAGE(STATUS)
MESSAGE(STATUS "---------------------------------------------------------------------------" )
MESSAGE(STATUS "CMAKE_BUILD_TYPE     = ${uppercase_CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "CMAKE_C_COMPILER     = ${CMAKE_C_COMPILER}")
MESSAGE(STATUS "CMAKE_CXX_COMPILER   = ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS "CMAKE_C_FLAGS        = ${CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS      = ${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}}")
MESSAGE(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE(STATUS "---------------------------------------------------------------------------" )
MESSAGE(STATUS)
