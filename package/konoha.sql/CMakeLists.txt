cmake_minimum_required(VERSION 2.6)
find_library(HAVE_LIBMYSQL NAMES mysqlclient)
message(${HAVE_LIBMYSQL})

find_program(MYSQL_CONFIG NAMES mysql_config)

if(MYSQL_CONFIG)

execute_process(
	COMMAND ${MYSQL_CONFIG} --include
	OUTPUT_VARIABLE MYSQL_INCLUDE_DIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "-I" "" HAVE_MYSQL_H ${MYSQL_INCLUDE_DIR})
message(${HAVE_MYSQL_H})

if(HAVE_MYSQL_H)
	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBMYSQL})
	set(PACKAGE_EXTRA_LIBRARY ${ExtraLibs})
	set(PACKAGE_SCRIPT_CODE sql_glue.k)
	set(PACKAGE_SOURCE_CODE
		sql_glue.c
		#src/mysql.c
		#src/sqlite.c
	)
	include_directories(${HAVE_MYSQL_H})
	add_konoha_package(konoha.sql)
endif(HAVE_MYSQL_H)
endif(MYSQL_CONFIG)

