# written by imasahiro

set(MODULE_SOURCE_CODE Compiler.c FuelVM.c codegen.c lowering.c visitor.c
        Optmizer.c Analysis.c)

set(LLVM_INSTALL_DIR "" CACHE STRING "Install dir which llvm installed")

FIND_PROGRAM(LLVM_CONFIG
   NAMES llvm-config
         llvm-config-3.1
         llvm-config-3.0
         llvm-config-2.9
         llvm-config-2.8
   PATHS ${LLVM_INSTALL_DIR}/bin/
         ${PROJECT_SOURCE_DIR}/llvm-3.2/bin/
         /usr/local/bin/
         /usr/bin/
   DOC "llvm-config")

if(LLVM_CONFIG)
    execute_process(
            COMMAND ${LLVM_CONFIG} --version
            OUTPUT_VARIABLE LLVM_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    execute_process(
            COMMAND ${LLVM_CONFIG} --includedir
            OUTPUT_VARIABLE LLVM_INCLUDEDIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    execute_process(
            COMMAND ${LLVM_CONFIG} --cxxflags
            OUTPUT_VARIABLE LLVM_CXXFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    execute_process(
            COMMAND ${LLVM_CONFIG} --ldflags
            OUTPUT_VARIABLE LLVM_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    execute_process(
            COMMAND ${LLVM_CONFIG} --libs core mcjit native linker ipo engine interpreter
            OUTPUT_VARIABLE LLVM_JIT_LIBS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    execute_process(
            COMMAND ${LLVM_CONFIG} --libdir
            OUTPUT_VARIABLE LLVM_LIBDIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

    set(ORIG_CXX_FLAGS ${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}})
    set(MODULE_CXX_FLAGS "${LLVM_CXXFLAGS} -Wno-variadic-macros ${ORIG_CXX_FLAGS} -UNDEBUG")
    string(REPLACE "-l" "" tmp ${LLVM_JIT_LIBS})
    string(REPLACE " " ";" LLVMLIBS ${tmp})
    string(REPLACE " " ";" tmp ${LLVM_LDFLAGS})

    foreach(lib ${tmp})
        string(REGEX MATCH "^-l" res "${lib}")
        if("${res}" STREQUAL "-l")
            string(REPLACE "-l" "" lib ${lib})
            set(LLVMLIBS ${LLVMLIBS} ${lib})
        endif()
    endforeach(lib ${tmp})

    set(MODULE_EXTRA_LIBRARY ${LLVMLIBS})
    link_directories(${LINK_DIRECTORIES} ${LLVM_LIBDIR})

    set(K_LLVM_VERSION ${LLVM_VERSION})
    string(REPLACE "." "0" K_LLVM_VERSION ${K_LLVM_VERSION})
    string(REPLACE "svn" "" K_LLVM_VERSION ${K_LLVM_VERSION})
    if(${LLVM_VERSION} VERSION_EQUAL "3.2")
        add_definitions(-DLLVM_VERSION=${K_LLVM_VERSION})
        add_definitions(-DFUELVM_USE_LLVM=1)
        include_directories(${LLVM_INCLUDEDIR})
        set(MODULE_SOURCE_CODE ${MODULE_SOURCE_CODE} LLVMCompiler.cpp)
    endif()
    add_konoha_module(FuelVM)
endif()
