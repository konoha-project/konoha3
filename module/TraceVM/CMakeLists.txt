cmake_minimum_required(VERSION 2.6)

check_include_file(db.h HAVE_DB_H)
find_library(HAVE_LIBDB NAMES db)
if(HAVE_LIBDB)
	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBDB})
	set(MODULE_EXTRA_LIBRARY ${ExtraLibs})
	set(MODULE_SOURCE_CODE TraceVM.c)
	add_konoha_module(TraceVM)
endif(HAVE_LIBDB)

