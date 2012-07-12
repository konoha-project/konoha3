cmake_minimum_required(VERSION 2.6)

project(dse)
set(CMAKE_C_FLAGS "-Wall")
option(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
option(CMAKE_C_FLAGS_RELEASE "-O2 -g0")

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIRS})

file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/dse.c)
set(DSE ${SRCS})

add_executable(${PROJECT_NAME} ${DSE})

set(LIBS event)
set(LIBS ${LIBS} jansson)
set(LIBS ${LIBS} konoha2)
set(LIBS ${LIBS} logpool)
target_link_libraries(${PROJECT_NAME} ${LIBS})
