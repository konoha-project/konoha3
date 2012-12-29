cmake_minimum_required(VERSION 2.6)
check_include_files(pcre.h HAVE_PCRE_H)
if(NOT HAVE_PCRE_H)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/missing)
endif(NOT HAVE_PCRE_H)

FIND_PACKAGE_EXTRA_LIBRARY(pcre)
set(PACKAGE_SOURCE_CODE Regexp_glue.c)
add_konoha_package(JavaScript.Regexp)
