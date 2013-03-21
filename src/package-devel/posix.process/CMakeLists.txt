if(NOT MINGW AND NOT MSVC)
	set(PACKAGE_SOURCE_CODE process_glue.c)
	add_konoha_package(posix.process)
endif()
