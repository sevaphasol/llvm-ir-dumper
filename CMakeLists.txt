cmake_minimum_required(VERSION 3.20)
project(graphcc LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GRAPHCC_INSTALL_SYMLINK "Create an extra graphcc symlink" OFF)
set(GRAPHCC_INSTALL_SYMLINK_DIR
    "$ENV{HOME}/.local/bin"
    CACHE PATH "Directory for optional graphcc symlink.")

find_package(LLVM REQUIRED CONFIG)
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(SOURCES_DIR "sources")
set(INCLUDE_DIR "include")
set(UTILS_DIR "sources/utils")
set(ARGPARSE_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/argparse/include")
set(DOT_GRAPH_LIB_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/dot-graph-lib")
set(NLOHMANN_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/nlohmann/include")

set(LLVM_IR_DUMPER_SOURCES
    ${SOURCES_DIR}/ir_graph/ir_graph.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_serde.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_builder.cpp
    ${SOURCES_DIR}/llvm_ir_dumper/dumper_pass.cpp
    ${SOURCES_DIR}/llvm_ir_dumper/pass_api.cpp
    ${SOURCES_DIR}/llvm_ir_dumper/inject_pass.cpp)

set(IR_GRAPH_TO_DOT_SOURCES
    ${SOURCES_DIR}/ir_graph/ir_graph.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_serde.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_to_dot.cpp
    ${SOURCES_DIR}/utils/ir_graph_to_dot_main.cpp)

set(IR_GRAPH_PROFILE_MERGE_SOURCES
    ${SOURCES_DIR}/ir_graph/ir_graph.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_serde.cpp
    ${SOURCES_DIR}/ir_graph/ir_graph_profile.cpp
    ${SOURCES_DIR}/utils/ir_graph_profile_merge_main.cpp)

advalidate_config(config)

reset_tmp_dird_library(LLVMIRDumper SHARED ${LLVM_IR_DUMPER_SOURCES})

target_include_directories(LLVMIRDumper PRIVATE ${INCLUDE_DIR}
                                                ${NLOHMANN_INCLUDE_DIR})

add_executable(ir_graph_to_dot ${IR_GRAPH_TO_DOT_SOURCES})

add_executable(ir_graph_profile_merge ${IR_GRAPH_PROFILE_MERGE_SOURCES})

target_include_directories(
  ir_graph_to_dot PRIVATE ${INCLUDE_DIR} ${ARGPARSE_INCLUDE_DIR}
                          ${DOT_GRAPH_LIB_INCLUDE_DIR} ${NLOHMANN_INCLUDE_DIR})

target_include_directories(
  ir_graph_profile_merge PRIVATE ${INCLUDE_DIR} ${ARGPARSE_INCLUDE_DIR}
                                 ${NLOHMANN_INCLUDE_DIR})

install(TARGETS LLVMIRDumper LIBRARY DESTINATION lib)

install(TARGETS ir_graph_to_dot ir_graph_profile_merge RUNTIME DESTINATION bin)

install(
  PROGRAMS ${UTILS_DIR}/graphcc.py
  DESTINATION bin
  RENAME graphcc)

if(GRAPHCC_INSTALL_SYMLINK)
  install(
    CODE "
file( MAKE_DIRECTORY \"$ENV{DESTDIR}${GRAPHCC_INSTALL_SYMLINK_DIR}\" )
file( REMOVE \"$ENV{DESTDIR}${GRAPHCC_INSTALL_SYMLINK_DIR}/graphcc\" )
file( REAL_PATH \"\${CMAKE_INSTALL_PREFIX}/bin/graphcc\" REAL_PATH_GRAPHCC )
file(
    CREATE_LINK
    \"\${REAL_PATH_GRAPHCC}\"
    \"$ENV{DESTDIR}${GRAPHCC_INSTALL_SYMLINK_DIR}/graphcc\"
    SYMBOLIC
)
")
endif()
