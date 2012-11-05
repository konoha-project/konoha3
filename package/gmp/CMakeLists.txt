cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(gmp)
set(PACKAGE_SOURCE_CODE gmp_glue.c)
set(PACKAGE_SCRIPT_CODE gmp_glue.k)
add_konoha_package(gmp)
