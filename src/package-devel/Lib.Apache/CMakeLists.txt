cmake_minimum_required(VERSION 2.6)
set(APACHE_LOCATION)
find_path(APACHE_INCLUDE_DIR
          NAMES httpd.h
          HINTS ${APACHE_LOCATION} ${APACHE_LOCATION}/include
          PATHS /usr/include/httpd
                /usr/include/apache2
                ${APACHE_LOCATION}
          DOC "Path to the apache include files"
)

find_path(APR_INCLUDE_DIR apr.h
        /usr/local/include/apr-1
        /usr/local/include/apr-1.0
        /usr/include/apr-1
        /usr/include/apr-1.0)

find_library(APR_LIBRARY
        NAMES apr-1
        PATHS /usr/local/lib /usr/lib)

if(APR_INCLUDE_DIR AND APR_LIBRARY AND APACHE_INCLUDE_DIR)
	include_directories(${APR_INCLUDE_DIR} ${APACHE_INCLUDE_DIR})
	set(PACKAGE_SOURCE_CODE apache_glue.c)
	set(PACKAGE_EXTRA_LIBRARY ${APR_LIBRARY})
	add_konoha_package(Lib.Apache)
else(APR_INCLUDE_DIR AND APR_LIBRARY AND APACHE_INCLUDE_DIR)
	show_package_warning(apache)
endif(APR_INCLUDE_DIR AND APR_LIBRARY AND APACHE_INCLUDE_DIR)
