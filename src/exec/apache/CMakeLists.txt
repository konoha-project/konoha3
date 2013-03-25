if(USE_APACHE_PLATFORM)
	project(mod_konoha)
	set(TARGET mod_konoha)
	set(SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mod_konoha.c)
	find_program(APXS_BIN
		NAMES   apxs apxs2
		PATHS /usr/local/httpd/bin /etc/httpd/bin)

	if(APXS_BIN)
		if(APPLE)
			set(MAKEFILE macosx)
		elseif(UNIX)
			if(EXISTS "/etc/issue")
				set(LINUX_NAME "")
				file(READ "/etc/issue" LINUX_ISSUE)
				if(LINUX_ISSUE MATCHES "CentOS")
					set(MAKEFILE centos)
				elseif(LINUX_ISSUE MATCHES "Debian")
					set(MAKEFILE ubuntu)
				elseif(LINUX_ISSUE MATCHES "Ubuntu")
					set(MAKEFILE ubuntu)
				endif()
			endif(EXISTS "/etc/issue")
		endif(APPLE)
		add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Makefile
			COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR}/${MAKEFILE}.mk
			${CMAKE_CURRENT_SOURCE_DIR}/Makefile)
		add_custom_target(link_mod_konoha_Makefile ALL DEPENDS
			${CMAKE_CURRENT_SOURCE_DIR}/Makefile
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/macosx.mk
			${CMAKE_CURRENT_SOURCE_DIR}/centos.mk
			${CMAKE_CURRENT_SOURCE_DIR}/ubuntu.mk)
		add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.la
			${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.lo
			${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.o
			${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.slo
			${CMAKE_CURRENT_SOURCE_DIR}/.libs
			COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}
			KONOHA_BUILD_DIR="${PACKAGE_BUILD_DIR}"
			DEPENDS ${SRCS})
		add_custom_target(mod_konoha ALL DEPENDS
			${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.la
			${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.lo
			${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.o
			${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.slo
			${CMAKE_CURRENT_SOURCE_DIR}/.libs)
		install(CODE "execute_process(COMMAND make -C
		${CMAKE_CURRENT_SOURCE_DIR} install)")
	endif(APXS_BIN)

endif(USE_APACHE_PLATFORM)
