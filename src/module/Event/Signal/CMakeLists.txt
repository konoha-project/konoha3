if(NOT MSVC AND NOT MINGW)
	set(MODULE_SOURCE_CODE Signal.c)
	add_konoha_module(Signal)
endif ()
