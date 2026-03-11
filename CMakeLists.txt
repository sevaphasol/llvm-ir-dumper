cmake_minimum_required( VERSION 3.20 )
project( llvm_ir_dumper LANGUAGES C CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

find_package( LLVM REQUIRED CONFIG )
include_directories( SYSTEM ${LLVM_INCLUDE_DIRS} )
add_definitions( ${LLVM_DEFINITIONS} )

set( SOURCES_DIR "sources" )
set( INCLUDE_DIR "include" )

set( LLVM_IR_DUMPER_SOURCES_DIR "${SOURCES_DIR}/llvm_ir_dumper" )
set( DOT_GRAPH_SOURCES_DIR      "${SOURCES_DIR}/dot_graph" )

file(
    GLOB_RECURSE
    LLVM_IR_DUMPER_SOURCES
    ${LLVM_IR_DUMPER_SOURCES_DIR}/*.cpp
)

file(
    GLOB_RECURSE
    DOT_GRAPH_SOURCES
    ${DOT_GRAPH_SOURCES_DIR}/*.cpp
)

add_library(
    LLVMIRDumper
    SHARED
    ${LLVM_IR_DUMPER_SOURCES}
    ${DOT_GRAPH_SOURCES}
)

target_include_directories(
    LLVMIRDumper
    PRIVATE
    ${INCLUDE_DIR}
)

install(
    TARGETS LLVMIRDumper
    LIBRARY DESTINATION lib
)
