cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(pthread)
set(PACKAGE_SOURCE_CODE thread_glue.c)
set(PACKAGE_SCRIPT_CODE thread_glue.k)
add_konoha_package(posix.thread)
