cmake_minimum_required( VERSION 3.20 )
project( llvm_pass_dumper LANGUAGES C CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

find_package( LLVM REQUIRED CONFIG )
include_directories( SYSTEM ${LLVM_INCLUDE_DIRS} )
add_definitions( ${LLVM_DEFINITIONS} )

set( LLVM_PASS_DUMPER_DIR dumper )
set( LLVM_PASS_DUMPER_SOURCES_DIR ${LLVM_PASS_DUMPER_DIR}/sources )
set( LLVM_PASS_DUMPER_INCLUDE_DIR ${LLVM_PASS_DUMPER_DIR}/include )

file(
    GLOB_RECURSE
    LLVM_PASS_DUMPER_SOURCES
    ${LLVM_PASS_DUMPER_SOURCES_DIR}/*.cpp
)

add_library(
    LLVMPassDumper
    SHARED
    ${LLVM_PASS_DUMPER_SOURCES}
)

target_include_directories(
    LLVMPassDumper
    PRIVATE
    ${LLVM_PASS_DUMPER_INCLUDE_DIR}
)

install(
    TARGETS LLVMPassDumper
    LIBRARY DESTINATION lib
)
