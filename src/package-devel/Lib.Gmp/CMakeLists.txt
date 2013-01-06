cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(gmp)
set(PACKAGE_SOURCE_CODE gmp_glue.c)
add_konoha_package(Lib.Gmp)
