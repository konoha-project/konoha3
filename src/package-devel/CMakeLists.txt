# package

macro(PKGNAME _STR _RESULT)
	set(_tmp)
	POP(${_STR} "/" _tmp)
	POP(${_tmp} "." ${_RESULT})
endmacro()

macro(show_package_warning)
	if(${ARGC} EQUAL 2)
		set(WARNINGS ${WARNINGS} "Disabled ${ARGV0} package because ${ARGV1} is not found." PARENT_SCOPE)
	else(${ARGC} EQUAL 2)
		set(WARNINGS ${WARNINGS} "Disabled ${ARGV0} package because dependency libraries are not found." PARENT_SCOPE)
	endif(${ARGC} EQUAL 2)
endmacro(show_package_warning)

macro(find_package_extra_library)
	foreach(_PKGLIB ${ARGN})
		find_library(HAVE_LIB${_PKGLIB} NAMES ${_PKGLIB})
		if(HAVE_LIB${_PKGLIB})
			set(PACKAGE_EXTRA_LIBRARY ${PACKAGE_EXTRA_LIBRARY} ${HAVE_LIB${_PKGLIB}})
		else(HAVE_LIB${_PKGLIB})
			set(__NOT_FOUND ${__NOT_FOUND} ${_PKGLIB})
			set(__NOT_BUILD true)
		endif(HAVE_LIB${_PKGLIB})
	endforeach(_PKGLIB ${ARGN})
endmacro(find_package_extra_library)

macro(add_konoha_package _PKGNAME)
	if(NOT __NOT_BUILD)
		project(${_PKGNAME})
		#message(STATUS "Compiling ${_PKGNAME} package")
		ENABLE_COVERAGE_INFO_IF_ENABLE(PACKAGE_EXTRA_LIBRARY)
		file(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/test/*.k)
		foreach(f ${test_files})
			string(REPLACE // / f ${f})
			string(REPLACE ${PARENT_SOURCE_DIR}/src/package/ "" test_name ${f})
			string(REPLACE ${PARENT_SOURCE_DIR}/src/package-devel/ "" test_name ${test_name})
			string(REPLACE /test "" test_name ${test_name})
			add_konoha_test(${test_name} ${f})
			if(PACKAGE_ENV)
				set_tests_properties(${test_name} PROPERTIES ENVIRONMENT ${PACKAGE_ENV})
			endif(PACKAGE_ENV)
		endforeach(f ${test_files})
		set(CMAKE_BUILD_TYPE ${KONOHA_BUILD_TYPE})
		PKGNAME(${_PKGNAME} _PACKAGE_NAME)
		set(LIB  "${_PACKAGE_NAME}_glue")
		set(_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
		set(_PACKAGE_STRING  ${CMAKE_PROJECT_NAME}-${_PACKAGE_VERSION})
		set(_KONOHA_PACKAGE_DIR lib/konoha/${SO_VERSION}/package/${KONOHA_VERSION}/${PROJECT_NAME})
		set(CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}
			"${CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}} ${PACKAGE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}
			"${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}} ${PACKAGE_CXX_FLAGS}")
		set(_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
				${CMAKE_CURRENT_SOURCE_DIR}/include  ${CMAKE_CURRENT_BINARY_DIR}
				${KONOHA_INCLUDE_DIRS}
				${_PACKAGE_INCLUDE_DIR})
		include_directories(${_INCLUDE_DIRS})
		list(LENGTH PACKAGE_SOURCE_CODE LEN)
		if(${LEN} GREATER 0)
			add_library(${LIB} SHARED ${PACKAGE_SOURCE_CODE})
			SET_CXX_PROPERTY_IF_MSVC(${LIB} ${PACKAGE_SOURCE_CODE})
			set_target_properties(${LIB} PROPERTIES PREFIX "")
			if(PACKAGE_EXTRA_LIBRARY)
				target_link_libraries(${LIB} ${PACKAGE_EXTRA_LIBRARY})
			endif(PACKAGE_EXTRA_LIBRARY)
			install(TARGETS ${LIB} DESTINATION ${_KONOHA_PACKAGE_DIR})
		endif(${LEN} GREATER 0)
		if(PACKAGE_SCRIPT_CODE)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE_SCRIPT_CODE}
				DESTINATION ${_KONOHA_PACKAGE_DIR})
			foreach(scriptcode ${PACKAGE_SCRIPT_CODE})
				add_custom_command(OUTPUT
					${CMAKE_CURRENT_BINARY_DIR}/${scriptcode}
					COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${CMAKE_CURRENT_SOURCE_DIR}/${scriptcode}
					${CMAKE_CURRENT_BINARY_DIR}/${scriptcode})
			endforeach(scriptcode ${PACKAGE_SCRIPT_CODE})
			add_custom_target(copy_${_PKGNAME}_scriptcode ALL DEPENDS
				${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_SCRIPT_CODE})
		endif(PACKAGE_SCRIPT_CODE)
		# for copy test files
		if(PACKAGE_TEST_FILE)
			foreach(testfile ${PACKAGE_TEST_FILE})
				add_custom_command(OUTPUT
					${CMAKE_CURRENT_BINARY_DIR}/${testfile}
					COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${CMAKE_CURRENT_SOURCE_DIR}/test/${testfile}
					${CMAKE_CURRENT_BINARY_DIR}/${testfile})
			endforeach(testfile ${PACKAGE_TEST_FILE})
			add_custom_target(copy_${_PKGNAME}_testfile ALL DEPENDS
				${PACKAGE_TEST_FILE})
		endif(PACKAGE_TEST_FILE)
		add_custom_command(OUTPUT ${PARENT_BINARY_DIR}/package/${_PKGNAME}
			COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_CURRENT_BINARY_DIR}
			${PARENT_BINARY_DIR}/package/${_PKGNAME})
		add_custom_target(create_symlink_${_PKGNAME} ALL DEPENDS
			${PARENT_BINARY_DIR}/package/${_PKGNAME})
	else(NOT __NOT_BUILD)
		foreach(_PKGLIB ${__NOT_FOUND})
			show_package_warning(${_PKGNAME} ${_PKGLIB})
		endforeach(_PKGLIB ${__NOT_FOUND})
	endif(NOT __NOT_BUILD)
endmacro(add_konoha_package)

file(GLOB PACKAGES ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(dir ${PACKAGES})
	if(IS_DIRECTORY ${dir} AND EXISTS ${dir}/CMakeLists.txt)
		add_subdirectory(${dir})
	endif()
endforeach()
