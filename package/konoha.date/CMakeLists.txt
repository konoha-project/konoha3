cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(m)
set(PACKAGE_SOURCE_CODE date_glue.c)
set(PACKAGE_SCRIPT_CODE date_glue.k)
add_konoha_package(konoha.date)
