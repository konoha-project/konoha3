cmake_minimum_required(VERSION 2.6)
find_library(HAVE_LIBMYSQL NAMES mysqlclient)
find_library(HAVE_LIBSQLITE NAMES sqlite3)

find_program(MYSQL_CONFIG NAMES mysql_config)
find_program(PG_CONFIG NAMES pg_config)

if(MYSQL_CONFIG OR HAVE_LIBSQLITE OR PG_CONFIG)
	if(MYSQL_CONFIG)
	execute_process(
		COMMAND ${MYSQL_CONFIG} --include
		OUTPUT_VARIABLE MYSQL_INCLUDE_DIR
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	string(REPLACE "-I" "" HAVE_MYSQL_H ${MYSQL_INCLUDE_DIR})
		if(HAVE_MYSQL_H)
			set(ExtraLibs ${ExtraLibs} ${HAVE_LIBMYSQL})
			set(PACKAGE_EXTRA_LIBRARY ${ExtraLibs})
			set(PACKAGE_SCRIPT_CODE sql_glue.k)
			set(PACKAGE_SOURCE_CODE ${PACKAGE_SOURCE_CODE}
			  include/mysql.h
			)
			include_directories(${HAVE_MYSQL_H})
		set(KonohaPkgUsing_MYSQL 1)
		endif(HAVE_MYSQL_H)
	endif(MYSQL_CONFIG)

	if(HAVE_LIBSQLITE)
		set(ExtraLibs ${ExtraLibs} ${HAVE_LIBSQLITE})
		set(PACKAGE_EXTRA_LIBRARY ${ExtraLibs})
		set(PACKAGE_SOURCE_CODE ${PACKAGE_SOURCE_CODE}
			include/sqlite.h
		)
		set(KonohaPkgUsing_SQLITE 1)
	endif(HAVE_LIBSQLITE)

	if(PG_CONFIG)
		execute_process(
			COMMAND ${PG_CONFIG}
			OUTPUT_VARIABLE PG_CONFIG_RESULTS
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		string(REGEX MATCH "/[^=]+/include" PG_INCLUDE_DIR ${PG_CONFIG_RESULTS})
		string(REGEX MATCH "/[^=]+/server" PG_INCLUDE_DIR_SERVER ${PG_CONFIG_RESULTS})
		string(REGEX MATCH "/[^=]+/lib" PG_LIB_DIR ${PG_CONFIG_RESULTS})
		find_library(HAVE_LIBPOSTRESQL NAMES pq
				HINTS ${PG_LIB_DIR})
		set(ExtraLibs ${ExtraLibs} ${HAVE_LIBPOSTRESQL})
		set(PACKAGE_EXTRA_LIBRARY ${ExtraLibs})
		set(PACKAGE_SOURCE_CODE ${PACKAGE_SOURCE_CODE}
			include/postgresql.h
		)
		set(PG_INCLUDE_DIR_ALL ${PG_INCLUDE_DIR} ${PG_INCLUDE_DIR_SERVER})
		set(KonohaPkgUsing_PSQL 1)
		include_directories(${PG_INCLUDE_DIR_ALL})
	endif(PG_CONFIG)

	configure_file (
		"${CMAKE_CURRENT_SOURCE_DIR}/konoha_sql.config.h.in"
		"${CMAKE_CURRENT_BINARY_DIR}/konoha_sql.config.h"
	)

	include_directories(${CMAKE_CURRENT_BINARY_DIR})
	set(PACKAGE_SOURCE_CODE ${PACKAGE_SOURCE_CODE}
		sql_glue.c
	)
	add_konoha_package(konoha.sql)
else(MYSQL_CONFIG OR HAVE_LIBSQLITE OR PG_CONFIG)
	show_package_warning(konoha.sql)
endif(MYSQL_CONFIG OR HAVE_LIBSQLITE OR PG_CONFIG)
