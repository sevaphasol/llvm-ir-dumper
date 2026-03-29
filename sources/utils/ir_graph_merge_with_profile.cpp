#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "argparse/argparse.hpp"

#include "ir_graph/ir_graph_profile.h"
#include "ir_graph/ir_graph_serde.h"

namespace {

void
configureProgram( argparse::ArgumentParser& program )
{
    program.add_description(
        "Merge a static IR graph JSON with runtime [ir-log] events captured from an "
        "instrumented binary." );
    program.add_argument( "--input-json" ).required().help( "Path to the static graph JSON." );
    program.add_argument( "--runtime-log" )
        .required()
        .help( "Path to the captured program stdout with [ir-log] events." );
    program.add_argument( "--output-json" ).required().help( "Path to the merged output JSON." );
}

std::string
readWholeFile( const std::string& path )
{
    std::ifstream input( path );
    if ( !input )
    {
        throw std::runtime_error( "unable to open file: " + path );
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

} // namespace

int
main( int argc, char** argv )
try
{
    argparse::ArgumentParser program( "ir_graph_profile_merge" );
    configureProgram( program );
    program.parse_args( argc, argv );

    const auto input_json  = program.get<std::string>( "--input-json" );
    const auto runtime_log = program.get<std::string>( "--runtime-log" );
    const auto output_json = program.get<std::string>( "--output-json" );

    std::ifstream input( input_json );
    if ( !input )
    {
        throw std::runtime_error( "unable to open input json: " + input_json );
    }

    auto graph = ir_graph::deserialize( input );

    const auto runtime_output = readWholeFile( runtime_log );
    const auto profile        = ir_graph::parseRuntimeProfile( runtime_output );
    if ( profile.empty() )
    {
        throw std::runtime_error( "no runtime profiling events were captured in: " +
                                  runtime_log );
    }

    ir_graph::applyRuntimeProfile( graph, profile );

    std::ofstream output( output_json );
    if ( !output )
    {
        throw std::runtime_error( "unable to open output json: " + output_json );
    }

    output << ir_graph::serialize( graph );
    return 0;
} catch ( const std::exception& ex )
{
    std::cerr << "[ir-graph-profile-merge] " << ex.what() << '\n';
    return 1;
}
