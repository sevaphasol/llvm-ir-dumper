#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include <argparse/argparse.hpp>

#include "ir_graph/ir_graph_serde.h"
#include "ir_graph/ir_graph_to_dot.h"

namespace {

void
configureProgram( argparse::ArgumentParser& program )
{
    program.add_description(
        "Deserialize an IR graph JSON snapshot and render it as a DOT graph." );
    program.add_argument( "--input-json" )
        .required()
        .help( "Path to the serialized graph JSON." );
    program.add_argument( "--output-dot" ).required().help( "Path to the output DOT file." );
}

} // namespace

int
main( int argc, char** argv )
try
{
    argparse::ArgumentParser program( "ir_graph_to_dot" );
    configureProgram( program );
    program.parse_args( argc, argv );

    const auto input_json = program.get<std::string>( "--input-json" );
    const auto output_dot = program.get<std::string>( "--output-dot" );

    std::ifstream input( input_json );
    if ( !input )
    {
        throw std::runtime_error( "unable to open input json: " + input_json );
    }

    const auto graph = ir_graph::deserialize( input );

    std::ofstream output( output_dot );
    if ( !output )
    {
        throw std::runtime_error( "unable to open output dot: " + output_dot );
    }

    output << ir_graph::renderDotGraph( graph );
    return 0;
} catch ( const std::exception& ex )
{
    std::cerr << "[ir-graph-to-dot] " << ex.what() << '\n';
    return 1;
}
