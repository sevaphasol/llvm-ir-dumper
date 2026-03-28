add_subdirectory(argparse)
add_subdirectory(dot-graph-lib)
add_subdirectory(nlohmann)

if(TARGET DotGraphLib AND NOT TARGET dot_graph_lib::dot_graph_lib)
    add_library(dot_graph_lib::dot_graph_lib ALIAS DotGraphLib)
endif()
