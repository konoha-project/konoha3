## written by tetz
set(MODULE_SOURCE_CODE JSGen.cpp)
find_library(HAVE_LIBV8 NAMES v8)
if(HAVE_LIBV8)
	set(MODULE_EXTRA_LIBRARY ${HAVE_LIBV8})
endif(HAVE_LIBV8)

add_konoha_module(JSGen)
