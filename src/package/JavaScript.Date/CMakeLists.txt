cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(m)
set(PACKAGE_SOURCE_CODE Date_glue.c)
add_konoha_package(JavaScript.Date)
