cmake_minimum_required(VERSION 2.6)
set(PACKAGE_SOURCE_CODE shell_glue.c)
set(PACKAGE_SCRIPT_CODE shell_glue.k shell_kick.k)
add_konoha_package(dscript.shell)
