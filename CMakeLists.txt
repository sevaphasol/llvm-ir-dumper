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
set( NLOHMANN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/nlohmann/include" )

set(
    LLVM_IR_DUMPER_SOURCES
    ${SOURCES_DIR}/ir_graph/ir_graph.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_serde.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_builder.cpp
    ${SOURCES_DIR}/llvm_ir_dumper/dumper_pass.cpp
    ${SOURCES_DIR}/llvm_ir_dumper/pass_api.cpp
)

set(
    IR_GRAPH_TO_DOT_SOURCES
    ${SOURCES_DIR}/ir_graph/ir_graph.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_serde.cpp
    ${SOURCES_DIR}/dot_graph/dot_graph.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_to_dot.cpp
    ${SOURCES_DIR}/tools/ir_graph_to_dot_main.cpp
)

add_library(
    LLVMIRDumper
    SHARED
    ${LLVM_IR_DUMPER_SOURCES}
)

target_include_directories(
    LLVMIRDumper
    PRIVATE
    ${INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
)

add_executable(
    ir_graph_to_dot
    ${IR_GRAPH_TO_DOT_SOURCES}
)

target_include_directories(
    ir_graph_to_dot
    PRIVATE
    ${INCLUDE_DIR}
    ${NLOHMANN_INCLUDE_DIR}
)

install(
    TARGETS LLVMIRDumper
    LIBRARY DESTINATION lib
)

install(
    TARGETS ir_graph_to_dot
    RUNTIME DESTINATION bin
)
