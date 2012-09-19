include(CheckTypeSize)
include(CheckIncludeFiles)

cmake_minimum_required(VERSION 2.4)
project(kjson)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

SET(CMAKE_C_FLAGS         "-Wall -Wno-variadic-macros")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -g")
SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
string(TOUPPER ${CMAKE_BUILD_TYPE} uppercase_CMAKE_BUILD_TYPE)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DNDEBUG)
endif()


set(JSON_SRC kjson.c)
add_library(kjson SHARED ${JSON_SRC})

# jemalloc
SET(SEARCH_DIRS
    /usr/local
    /opt/local # DarwinPorts
)

find_path(JEMALLOC_INCLUDE_DIR NAMES jemalloc.h
    HINTS ${SEARCH_DIRS} PATH_SUFFIXES include/jemalloc)

find_library(JEMALLOC_LIBRARY NAMES jemalloc
    HINTS ${SEARCH_DIRS} PATH_SUFFIXES lib64 lib)

if(JEMALLOC_INCLUDE_DIR AND JEMALLOC_LIBRARY)
    add_definitions(-DHAVE_JEMALLOC_H=1)
    target_link_libraries(kjson ${JEMALLOC_LIBRARY})
endif(JEMALLOC_INCLUDE_DIR AND JEMALLOC_LIBRARY)

## Test cases
set(JSON_TEST
        "./test/test01.json"
        "./test/test02.json"
        "./test/test03.json"
        "./test/test04.json"
        "./test/test05.json"
        "./test/test06.json"
        "./test/test07.json"
        "./test/test08.json"
        "./test/test09.json"
        "./test/test10.json"
        "./test/test11.json"
        "./test/test12.json"
        "./test/test13.json"
        "./test/test14.json"
        "./test/test15.json")

set(JSON_BENCHMARK
        "./test/benchmark1.json"
        "./test/benchmark2.json"
        "./test/benchmark3.json"
        "./test/benchmark4.json"
        "./test/benchmark5.json"
        "./test/benchmark6.json"
        "./test/benchmark7.json")


set(JSON_TEST_CODE_LIST)
set(JSON_BENCH_CODE_LIST)
foreach(it ${JSON_TEST})
    set(JSON_TEST_CODE_LIST ${JSON_TEST_CODE_LIST}
            ${CMAKE_CURRENT_SOURCE_DIR}/${it})
endforeach()

foreach(it ${JSON_BENCH})
    set(JSON_BENCH_CODE_LIST ${JSON_BENCH_CODE_LIST}
            ${CMAKE_CURRENT_SOURCE_DIR}/${it})
endforeach()

enable_testing()
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

add_executable(test_tostring test/test_tostring.c)
target_link_libraries(test_tostring kjson)
add_test(kjson_test_tostring test_tostring)

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

add_executable(parseString test/parseString.c)
add_test(kjson_parseString parseString)
target_link_libraries(parseString kjson)

add_executable(check_cpp test/check_cpp_build.cpp)
add_test(kjson_check_cpp check_cpp)

add_executable(test_whitespace test/test_whitespace.c)
add_test(kjson_test_whitespace test_whitespace)
target_link_libraries(test_whitespace kjson)

install(TARGETS kjson LIBRARY DESTINATION lib)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kjson.h DESTINATION include/kjson)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/numbox.h DESTINATION include/kjson)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/map.h DESTINATION include/kjson)

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
