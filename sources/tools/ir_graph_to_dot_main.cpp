#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "ir_graph/ir_graph_serde.h"
#include "ir_graph/ir_graph_to_dot.h"

namespace {

struct CliArgs
{
    std::string input_json;
    std::string output_dot;
};

void
printUsage( std::string_view program_name )
{
    std::cerr << "Usage: " << program_name << " --input-json <path> --output-dot <path>\n";
}

CliArgs
parseArgs( int argc, char** argv )
{
    CliArgs args;

    for ( int i = 1; i < argc; ++i )
    {
        const std::string_view arg = argv[i];

        if ( arg == "--input-json" && i + 1 < argc )
        {
            args.input_json = argv[++i];
            continue;
        }

        if ( arg == "--output-dot" && i + 1 < argc )
        {
            args.output_dot = argv[++i];
            continue;
        }

        if ( arg == "--help" || arg == "-h" )
        {
            printUsage( argv[0] );
            std::exit( 0 );
        }

        throw std::runtime_error( "unknown argument: " + std::string( arg ) );
    }

    if ( args.input_json.empty() || args.output_dot.empty() )
    {
        throw std::runtime_error( "both --input-json and --output-dot are required" );
    }

    return args;
}

} // namespace

int
main( int argc, char** argv )
{
    try
    {
        const CliArgs args = parseArgs( argc, argv );

        std::ifstream input( args.input_json );
        if ( !input )
        {
            throw std::runtime_error( "unable to open input json: " + args.input_json );
        }

        const auto graph = ir_graph::deserialize( input );

        std::ofstream output( args.output_dot );
        if ( !output )
        {
            throw std::runtime_error( "unable to open output dot: " + args.output_dot );
        }

        output << llvm_ir_dumper::renderDotGraph( graph );
    } catch ( const std::exception& ex )
    {
        std::cerr << "[ir-graph-to-dot] " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
