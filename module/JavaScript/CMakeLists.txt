## written by tetz
find_library(HAVE_LIBV8 NAMES v8)
if(HAVE_LIBV8)
	set(MODULE_SOURCE_CODE JavaScript.cpp)
	set(MODULE_EXTRA_LIBRARY ${HAVE_LIBV8})
	add_konoha_module(JavaScript)
endif(HAVE_LIBV8)

