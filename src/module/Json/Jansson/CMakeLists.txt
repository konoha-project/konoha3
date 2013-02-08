cmake_minimum_required(VERSION 2.6)
find_library(HAVE_LIBJANSSON NAMES jansson)
if(HAVE_LIBJANSSON)
	set(MODULE_EXTRA_LIBRARY ${HAVE_LIBJANSSON})
	set(MODULE_SOURCE_CODE Jansson.c)
	add_konoha_module(Jansson)
endif(HAVE_LIBJANSSON)

