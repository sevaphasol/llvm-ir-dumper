#include "ir_graph/ir_graph_profile.h"

#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ir_graph {

namespace {

constexpr std::string_view kLogPrefix = "[ir-log] ";

bool
startsWith( std::string_view text, std::string_view prefix )
{
    return text.size() >= prefix.size() && text.substr( 0, prefix.size() ) == prefix;
}

std::string_view
trimCarriageReturn( std::string_view text )
{
    if ( !text.empty() && text.back() == '\r' )
    {
        text.remove_suffix( 1 );
    }

    return text;
}

std::uint64_t
parseUnsigned( std::string_view value, std::string_view field_name )
{
    std::uint64_t result = 0;
    const auto*   begin  = value.data();
    const auto*   end    = value.data() + value.size();
    const auto    parse  = std::from_chars( begin, end, result );

    if ( parse.ec != std::errc{} || parse.ptr != end )
    {
        throw std::runtime_error( "invalid numeric value for field '" + std::string( field_name ) +
                                  "': " + std::string( value ) );
    }

    return result;
}

std::uint64_t
requireUnsignedField( std::string_view fields,
                      std::string_view key,
                      std::string_view event_name,
                      std::string_view line )
{
    std::size_t cursor = 0;

    while ( cursor < fields.size() )
    {
        const auto next_space = fields.find( ' ', cursor );
        const auto token =
            fields.substr( cursor, next_space == std::string_view::npos ? fields.size() - cursor
                                                                        : next_space - cursor );
        const auto equals_pos = token.find( '=' );

        if ( equals_pos != std::string_view::npos && token.substr( 0, equals_pos ) == key )
        {
            return parseUnsigned( token.substr( equals_pos + 1 ), key );
        }

        if ( next_space == std::string_view::npos )
        {
            break;
        }

        cursor = next_space + 1;
    }

    throw std::runtime_error( "missing field '" + std::string( key ) + "' in runtime event '" +
                              std::string( event_name ) + "': " + std::string( line ) );
}

void
parseRuntimeProfileLine( RuntimeProfile& profile, std::string_view raw_line )
{
    const auto line = trimCarriageReturn( raw_line );
    if ( !startsWith( line, kLogPrefix ) )
    {
        return;
    }

    const auto payload    = line.substr( kLogPrefix.size() );
    const auto event_end  = payload.find( ' ' );
    const auto event_name = payload.substr( 0, event_end );
    const auto fields =
        event_end == std::string_view::npos ? std::string_view{} : payload.substr( event_end + 1 );

    if ( event_name == "func_enter" )
    {
        ++profile.function_execution_counts[requireUnsignedField( fields, "fid", event_name, line )];
        return;
    }

    if ( event_name == "bb_enter" )
    {
        ++profile.basic_block_execution_counts[requireUnsignedField( fields,
                                                                     "bbid",
                                                                     event_name,
                                                                     line )];
        return;
    }

    if ( event_name == "cfg_edge" || event_name == "call_edge" )
    {
        ++profile.edge_execution_counts[requireUnsignedField( fields, "eid", event_name, line )];
        return;
    }

    throw std::runtime_error( "unknown runtime event: " + std::string( event_name ) );
}

void
initializeProfileFields( ir_graph::Graph& graph )
{
    for ( auto function_id = ir_graph::Id{ 0 }; function_id < graph.functions().size();
          ++function_id )
    {
        auto& function = graph.function( function_id );
        function.executionCount( function.entryBasicBlockId().has_value() ? std::optional<std::uint64_t>{ 0 }
                                                                          : std::nullopt );
    }

    for ( auto basic_block_id = ir_graph::Id{ 0 }; basic_block_id < graph.basicBlocks().size();
          ++basic_block_id )
    {
        graph.basicBlock( basic_block_id ).executionCount( 0 );
    }

    for ( auto edge_id = ir_graph::Id{ 0 }; edge_id < graph.edges().size(); ++edge_id )
    {
        auto& edge = graph.edge( edge_id );
        const bool is_profiled_edge =
            edge.kind() == ir_graph::EdgeKind::ControlFlow || edge.kind() == ir_graph::EdgeKind::Call;
        edge.executionCount( is_profiled_edge ? std::optional<std::uint64_t>{ 0 } : std::nullopt );
    }
}

} // namespace

bool
RuntimeProfile::empty() const
{
    return function_execution_counts.empty() && basic_block_execution_counts.empty() &&
           edge_execution_counts.empty();
}

RuntimeProfile
parseRuntimeProfile( std::string_view runtime_output )
{
    RuntimeProfile profile;

    std::size_t line_begin = 0;
    while ( line_begin <= runtime_output.size() )
    {
        const auto line_end = runtime_output.find( '\n', line_begin );
        const auto line     = runtime_output.substr(
            line_begin,
            line_end == std::string_view::npos ? runtime_output.size() - line_begin
                                               : line_end - line_begin );

        if ( !line.empty() )
        {
            parseRuntimeProfileLine( profile, line );
        }

        if ( line_end == std::string_view::npos )
        {
            break;
        }

        line_begin = line_end + 1;
    }

    return profile;
}

void
applyRuntimeProfile( ir_graph::Graph& graph, const RuntimeProfile& profile )
{
    initializeProfileFields( graph );

    for ( const auto& [function_id, execution_count] : profile.function_execution_counts )
    {
        if ( function_id >= graph.functions().size() )
        {
            throw std::runtime_error( "runtime profile references unknown function id " +
                                      std::to_string( function_id ) );
        }

        graph.function( function_id ).executionCount( execution_count );
    }

    for ( const auto& [basic_block_id, execution_count] : profile.basic_block_execution_counts )
    {
        if ( basic_block_id >= graph.basicBlocks().size() )
        {
            throw std::runtime_error( "runtime profile references unknown basic block id " +
                                      std::to_string( basic_block_id ) );
        }

        graph.basicBlock( basic_block_id ).executionCount( execution_count );
    }

    for ( const auto& [edge_id, execution_count] : profile.edge_execution_counts )
    {
        if ( edge_id >= graph.edges().size() )
        {
            throw std::runtime_error( "runtime profile references unknown edge id " +
                                      std::to_string( edge_id ) );
        }

        auto& edge = graph.edge( edge_id );
        const bool is_profiled_edge =
            edge.kind() == ir_graph::EdgeKind::ControlFlow || edge.kind() == ir_graph::EdgeKind::Call;
        if ( !is_profiled_edge )
        {
            throw std::runtime_error( "runtime profile references non-profiled edge id " +
                                      std::to_string( edge_id ) );
        }

        edge.executionCount( execution_count );
    }
}

} // namespace ir_graph
