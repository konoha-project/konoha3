set(SUFFIX_ARCH "")
if(CPACK_GENERATOR STREQUAL "PackageMaker") # macosx (dmg)
	set(SUFFIX_ARCH "_macosx")
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/COPYING ${CMAKE_CURRENT_SOURCE_DIR}/README
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/COPYING
		${CMAKE_CURRENT_BINARY_DIR}/COPYING.txt)
	file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/README
		${CMAKE_CURRENT_BINARY_DIR}/README.txt)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY /usr/local)
	set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_BINARY_DIR}/COPYING.txt)
	set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_BINARY_DIR}/README.txt)
elseif(CPACK_GENERATOR STREQUAL "DEB") # linux (deb)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY /usr)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(SUFFIX_ARCH "_amd64")
	else(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(SUFFIX_ARCH "_i386")
	endif(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS
		"libc6 (>= 2.11), libpcre3-dev")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER
		"Konoha Developers <admin@konohascript.org>")
elseif(CPACK_GENERATOR STREQUAL "RPM") # linux (rpm)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY /usr)
	set(CPACK_RPM_PACKAGE_LICENSE "Simplified BSD")
	set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
	set(CPACK_RPM_PACKAGE_URL "http://konohascript.org")
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(SUFFIX_ARCH "_x86_64")
	else(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(SUFFIX_ARCH "_i386")
	endif(CMAKE_SIZEOF_VOID_P MATCHES "8")
endif(CPACK_GENERATOR STREQUAL "PackageMaker")
set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_STRING}${SUFFIX_ARCH})

include(CPack)
