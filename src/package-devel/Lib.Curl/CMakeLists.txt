cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(curl)
set(PACKAGE_SOURCE_CODE Curl_glue.c)
add_konoha_package(Lib.Curl)
