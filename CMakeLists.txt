cmake_minimum_required(VERSION 3.20)
project(cdfgg LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(
    CDFGG_INSTALL_SYMLINK
    "Create an extra cdfgg symlink"
    OFF)
set(CDFGG_INSTALL_SYMLINK_DIR
    "$ENV{HOME}/.local/bin"
    CACHE PATH "Directory for optional cdfgg symlink.")

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
    CDFGG_DRIVER_SOURCES
    CONFIGURE_DEPENDS
    "${UTILS_SOURCES_DIR}/cdfgg.py")

add_library(
    cdfgg_llvm_support INTERFACE)
target_include_directories(
    cdfgg_llvm_support SYSTEM
    INTERFACE ${LLVM_INCLUDE_DIRS})
target_compile_options(cdfgg_llvm_support INTERFACE ${LLVM_DEFINITION_FLAGS})

add_library(
    cdfgg_ir_graph STATIC
    ${IR_GRAPH_SOURCES})
target_include_directories(
    cdfgg_ir_graph
    PUBLIC ${INCLUDE_DIR})
target_link_libraries(
    cdfgg_ir_graph
    PUBLIC nlohmann_json::nlohmann_json
    PRIVATE dot_graph_lib::dot_graph_lib)

add_library(
    cdfgg_ir_graph_builder STATIC
    ${IR_GRAPH_BUILDER_SOURCES})
target_include_directories(
    cdfgg_ir_graph_builder
    PUBLIC ${INCLUDE_DIR})
target_link_libraries(cdfgg_ir_graph_builder PUBLIC cdfgg_ir_graph cdfgg_llvm_support)

add_library(
    cdfgg-llvm-pass SHARED
    ${LLVM_PASS_SOURCES})
target_include_directories(
    cdfgg-llvm-pass
    PRIVATE ${INCLUDE_DIR})
target_link_libraries(cdfgg-llvm-pass PRIVATE cdfgg_ir_graph_builder)

add_executable(
    ir_graph_to_dot
    ${IR_GRAPH_TO_DOT_SOURCES})
target_link_libraries(ir_graph_to_dot PRIVATE argparse::argparse cdfgg_ir_graph)

add_executable(
    ir_graph_merge_with_profile
    ${IR_GRAPH_MERGE_WITH_PROFILE_SOURCES})
target_link_libraries(ir_graph_merge_with_profile PRIVATE argparse::argparse cdfgg_ir_graph)

install(
    TARGETS cdfgg-llvm-pass
    LIBRARY DESTINATION lib)
install(
    TARGETS ir_graph_to_dot ir_graph_merge_with_profile
    RUNTIME DESTINATION bin)
install(
    PROGRAMS ${CDFGG_DRIVER_SOURCES}
    DESTINATION bin
    RENAME cdfgg)

if(CDFGG_INSTALL_SYMLINK)
    install(
        CODE "
file(MAKE_DIRECTORY \"$ENV{DESTDIR}${CDFGG_INSTALL_SYMLINK_DIR}\")
file(REMOVE \"$ENV{DESTDIR}${CDFGG_INSTALL_SYMLINK_DIR}/cdfgg\")
file(REAL_PATH \"\${CMAKE_INSTALL_PREFIX}/bin/cdfgg\" REAL_PATH_CDFGG)
file(
    CREATE_LINK
    \"\${REAL_PATH_CDFGG}\"
    \"$ENV{DESTDIR}${CDFGG_INSTALL_SYMLINK_DIR}/cdfgg\"
    SYMBOLIC
)
")
endif()
