# package

macro(PKGNAME _STR _RESULT)
	set(_tmp)
	POP(${_STR} "/" _tmp)
	POP(${_tmp} "." ${_RESULT})
endmacro()

macro(add_konoha_package _PKGNAME)
	project(${_PKGNAME})
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
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_BINARY_DIR}
		${PARENT_BINARY_DIR}/package/${_PKGNAME})
	add_custom_target(create_copy_${_PKGNAME} ALL DEPENDS
		${PARENT_BINARY_DIR}/package/${_PKGNAME})
endmacro(add_konoha_package)

set(LIBRARY_SUBDIRS C JS Konoha)
foreach(sub ${LIBRARY_SUBDIRS})
	file(GLOB PACKAGES ${CMAKE_CURRENT_SOURCE_DIR}/${sub}/*)
	foreach(dir ${PACKAGES})
		if(IS_DIRECTORY ${dir} AND EXISTS ${dir}/CMakeLists.txt)
			add_subdirectory(${dir})
		endif()
	endforeach()
endforeach(sub ${LIBRARY_SUBDIRS})
