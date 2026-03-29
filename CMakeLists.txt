cmake_minimum_required(VERSION 3.20)
project(graphcc LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(
    GRAPHCC_INSTALL_SYMLINK
    "Create an extra graphcc symlink"
    OFF)
set(GRAPHCC_INSTALL_SYMLINK_DIR
    "$ENV{HOME}/.local/bin"
    CACHE PATH "Directory for optional graphcc symlink.")

find_package(
    LLVM
    REQUIRED
    CONFIG)
separate_arguments(
    LLVM_DEFINITION_FLAGS
    NATIVE_COMMAND
    "${LLVM_DEFINITIONS}")

add_subdirectory(3rd_party)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources")
set(IR_GRAPH_SOURCES_DIR "${SOURCES_DIR}/ir_graph")
set(LLVM_PASS_SOURCES_DIR "${SOURCES_DIR}/llvm_pass")
set(UTILS_SOURCES_DIR "${SOURCES_DIR}/utils")

file(
    GLOB
    LLVM_PASS_SOURCES
    CONFIGURE_DEPENDS
    "${LLVM_PASS_SOURCES_DIR}/*.cpp")
file(
    GLOB
    IR_GRAPH_SOURCES
    CONFIGURE_DEPENDS
    "${IR_GRAPH_SOURCES_DIR}/ir_graph_profile.cpp"
    "${IR_GRAPH_SOURCES_DIR}/ir_graph_serde.cpp"
    "${IR_GRAPH_SOURCES_DIR}/ir_graph_to_dot.cpp"
    "${IR_GRAPH_SOURCES_DIR}/ir_graph.cpp")
file(
    GLOB
    IR_GRAPH_BUILDER_SOURCES
    CONFIGURE_DEPENDS
    "${IR_GRAPH_SOURCES_DIR}/ir_graph_builder.cpp")
file(
    GLOB
    IR_GRAPH_MERGE_WITH_PROFILE_SOURCES
    CONFIGURE_DEPENDS
    "${UTILS_SOURCES_DIR}/ir_graph_merge_with_profile.cpp")
file(
    GLOB
    IR_GRAPH_TO_DOT_SOURCES
    CONFIGURE_DEPENDS
    "${UTILS_SOURCES_DIR}/ir_graph_to_dot.cpp")
file(
    GLOB
    GRPAHCC_DRIVER_SOURCES
    CONFIGURE_DEPENDS
    "${UTILS_SOURCES_DIR}/graphcc.py")

add_library(
    graphcc_llvm_support INTERFACE)
target_include_directories(
    graphcc_llvm_support SYSTEM
    INTERFACE ${LLVM_INCLUDE_DIRS})
target_compile_options(graphcc_llvm_support INTERFACE ${LLVM_DEFINITION_FLAGS})

add_library(
    graphcc_ir_graph STATIC
    ${IR_GRAPH_SOURCES})
target_include_directories(
    graphcc_ir_graph
    PUBLIC ${INCLUDE_DIR})
target_link_libraries(
    graphcc_ir_graph
    PUBLIC nlohmann_json::nlohmann_json
    PRIVATE dot_graph_lib::dot_graph_lib)

add_library(
    graphcc_ir_graph_builder STATIC
    ${IR_GRAPH_BUILDER_SOURCES})
target_include_directories(
    graphcc_ir_graph_builder
    PUBLIC ${INCLUDE_DIR})
target_link_libraries(graphcc_ir_graph_builder PUBLIC graphcc_ir_graph graphcc_llvm_support)

add_library(
    graphcc-llvm-pass SHARED
    ${LLVM_PASS_SOURCES})
target_include_directories(
    graphcc-llvm-pass
    PRIVATE ${INCLUDE_DIR})
target_link_libraries(graphcc-llvm-pass PRIVATE graphcc_ir_graph_builder)

add_executable(
    ir_graph_to_dot
    ${IR_GRAPH_TO_DOT_SOURCES})
target_link_libraries(ir_graph_to_dot PRIVATE argparse::argparse graphcc_ir_graph)

add_executable(
    ir_graph_merge_with_profile
    ${IR_GRAPH_MERGE_WITH_PROFILE_SOURCES})
target_link_libraries(ir_graph_merge_with_profile PRIVATE argparse::argparse graphcc_ir_graph)

install(
    TARGETS graphcc-llvm-pass
    LIBRARY DESTINATION lib)
install(
    TARGETS ir_graph_to_dot ir_graph_merge_with_profile
    RUNTIME DESTINATION bin)
install(
    PROGRAMS ${GRPAHCC_DRIVER_SOURCES}
    DESTINATION bin
    RENAME graphcc)

if(GRAPHCC_INSTALL_SYMLINK)
    install(
        CODE "
file(MAKE_DIRECTORY \"$ENV{DESTDIR}${GRAPHCC_INSTALL_SYMLINK_DIR}\")
file(REMOVE \"$ENV{DESTDIR}${GRAPHCC_INSTALL_SYMLINK_DIR}/graphcc\")
file(REAL_PATH \"\${CMAKE_INSTALL_PREFIX}/bin/graphcc\" REAL_PATH_GRAPHCC)
file(
    CREATE_LINK
    \"\${REAL_PATH_GRAPHCC}\"
    \"$ENV{DESTDIR}${GRAPHCC_INSTALL_SYMLINK_DIR}/graphcc\"
    SYMBOLIC
)
")
endif()
