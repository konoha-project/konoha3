cmake_minimum_required(VERSION 2.6)
set(PACKAGE_SOURCE_CODE ijit_glue.c)
set(PACKAGE_SCRIPT_CODE ijit_glue.k)

set(LLVM_INSTALL_DIR "" CACHE STRING "Install dir which llvm installed")
set(LLVM_CONFIG_BIN "llvm-config")
if(LLVM_INSTALL_DIR)
set(LLVM_CONFIG_BIN "${LLVM_INSTALL_DIR}/bin/${LLVM_CONFIG_BIN}")
endif(LLVM_INSTALL_DIR)

find_program(LLVM_CONFIG NAMES ${LLVM_CONFIG_BIN} DOC "llvm-config")
if(LLVM_CONFIG)
execute_process(
		COMMAND ${LLVM_CONFIG} --version
		OUTPUT_VARIABLE LLVM_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

set(K_LLVM_VERSION ${LLVM_VERSION})
string(REPLACE "." "0" K_LLVM_VERSION ${K_LLVM_VERSION})
string(REPLACE "svn" "" K_LLVM_VERSION ${K_LLVM_VERSION})

add_konoha_package(ijit)

else(LLVM_CONFIG)
message(STATUS
	"ijit package needs konoha.llvm package.")
endif(LLVM_CONFIG)
