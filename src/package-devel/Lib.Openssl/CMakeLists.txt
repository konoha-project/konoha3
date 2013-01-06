cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE_EXTRA_LIBRARY(ssl crypto)
set(PACKAGE_SOURCE_CODE openssl_glue.c)
add_konoha_package(Lib.Openssl)
