cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(curl)
set(PACKAGE_SOURCE_CODE curl_glue.c)
set(PACKAGE_SCRIPT_CODE curl_glue.k)
add_konoha_package(curl)
