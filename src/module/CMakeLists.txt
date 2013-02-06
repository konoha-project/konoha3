cmake_minimum_required(VERSION 2.6)

macro(GET_MODULE_NAME _STR _RESULT)
	string(REPLACE "/" ";" _tmp ${_STR})
	list(REVERSE _tmp)
	list(GET _tmp 0 ${_RESULT})
endmacro()

set(KONOHA_MODULE_DIR
	"lib/konoha/${SO_VERSION}/module/${ModuleName}")

macro(add_konoha_module ModuleName)
	#MESSAGE(STATUS "Compiling ${ModuleName} Module")
	ENABLE_COVERAGE_INFO_IF_ENABLE(MODULE_EXTRA_LIBRARY)
	set(CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}   "${CMAKE_C_FLAGS_${uppercase_CMAKE_BUILD_TYPE}}   ${MODULE_C_FLAGS}")
	set(CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE} "${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}} ${MODULE_CXX_FLAGS}")
	add_library(${ModuleName} SHARED ${MODULE_SOURCE_CODE})
	target_link_libraries(${ModuleName} ${MODULE_EXTRA_LIBRARY})
	set_target_properties(${ModuleName} PROPERTIES PREFIX "")
	add_custom_command(OUTPUT ${PARENT_BINARY_DIR}/module/${ModuleName}
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		${CMAKE_CURRENT_BINARY_DIR}
		${PARENT_BINARY_DIR}/module/${ModuleName})
	add_custom_target(create_symlink_${ModuleName} ALL DEPENDS
		${PARENT_BINARY_DIR}/module/${ModuleName})
	install(TARGETS ${ModuleName} DESTINATION ${KONOHA_MODULE_DIR}/${ModuleName})
endmacro(add_konoha_module ModuleName)

file(GLOB dirs ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(dir ${dirs})
	set(ModuleName)
	GET_MODULE_NAME(${dir} ModuleName)
	if(IS_DIRECTORY ${dir} AND EXISTS ${dir}/CMakeLists.txt)
		add_subdirectory(${dir})
	endif()
endforeach()
