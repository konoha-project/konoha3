cmake_minimum_required(VERSION 2.6)
check_include_file(db.h HAVE_DB)
set(MODULE_SOURCE_CODE TraceVM.c)
add_konoha_module(TraceVM)
