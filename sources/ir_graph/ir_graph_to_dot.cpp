#include "ir_graph/ir_graph_to_dot.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include <dot_graph/graph.h>

namespace ir_graph {

namespace {

struct EdgeStyle
{
    std::string_view color;
    std::size_t      weight;
    std::size_t      pen_width;
    std::string_view style      = "solid";
    bool             constraint = true;
};

struct NodeStyle
{
    std::string_view fill_color;
    std::string_view color;
    std::string_view font_color;
};

struct ClusterStyle
{
    std::string_view fill_color;
    std::string_view color;
    std::size_t      pen_width;
    std::size_t      font_size;
    std::string_view font_name;
};

constexpr EdgeStyle kControlFlowEdge = {
    .color     = "#ff8800",
    .weight    = 10000,
    .pen_width = 5,
};

constexpr EdgeStyle kInstructionSequenceEdge = {
    .color     = "#ff3636",
    .weight    = 1000,
    .pen_width = 3,
};

constexpr EdgeStyle kDataFlowEdge = {
    .color     = "#00b3ff",
    .weight    = 1,
    .pen_width = 2,
};

constexpr EdgeStyle kCallEdge = {
    .color      = "#ff8800",
    .weight     = 1,
    .pen_width  = 5,
    .style      = "dashed",
    .constraint = false,
};

constexpr NodeStyle kInstructionNode = {
    .fill_color = "#6d95df",
    .color      = "#000000",
    .font_color = "#000000",
};

constexpr NodeStyle kOperandNode = {
    .fill_color = "#d66868",
    .color      = "#000000",
    .font_color = "#000000",
};

constexpr ClusterStyle kFunctionCluster = {
    .fill_color = "#e3e3e3",
    .color      = "#000000",
    .pen_width  = 2,
    .font_size  = 20,
    .font_name  = "SF Pro Text Bold 10",
};

constexpr ClusterStyle kBasicBlockCluster = {
    .fill_color = "#a4a4a4",
    .color      = "#000000",
    .pen_width  = 2,
    .font_size  = 15,
    .font_name  = "SF Pro Text Bold 10",
};

struct RgbColor
{
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
};

constexpr RgbColor kColdBasicBlockColor = { 0x56, 0xc7, 0xc3 };
constexpr RgbColor kHotBasicBlockColor  = { 0xf4, 0xa2, 0x61 };

std::string
getNodeRenderId( ir_graph::Id node_id )
{
    return "node_" + std::to_string( node_id );
}

std::string
getFunctionClusterId( ir_graph::Id function_id )
{
    return "cluster_func_" + std::to_string( function_id );
}

std::string
getBasicBlockClusterId( ir_graph::Id function_id, ir_graph::Id basic_block_id )
{
    return "cluster_func_" + std::to_string( function_id ) + "_bb_" +
           std::to_string( basic_block_id );
}

void
applyEdgeStyle( dot_graph::Edge& edge, const EdgeStyle& style )
{
    edge.setColor( style.color )
        .setWeight( style.weight )
        .setPenWidth( style.pen_width )
        .setStyle( style.style )
        .setConstraint( style.constraint );
}

void
applyNodeStyle( dot_graph::Node& node, const NodeStyle& style )
{
    node.setShape( "box" )
        .setStyle( "rounded, filled" )
        .setFillColor( style.fill_color )
        .setColor( style.color )
        .setFontColor( style.font_color );
}

void
applyClusterStyle( dot_graph::Subgraph& cluster, const ClusterStyle& style )
{
    cluster.setShape( "box" )
        .setStyle( "rounded, filled" )
        .setFillColor( style.fill_color )
        .setColor( style.color )
        .setPenWidth( style.pen_width )
        .setFontSize( style.font_size )
        .setFontName( style.font_name );
}

std::string
formatFunctionLabel( const ir_graph::Function& function )
{
    if ( !function.executionCount().has_value() )
    {
        return function.name();
    }

    return function.name() + "\\ncalls=" + std::to_string( *function.executionCount() );
}

std::string
formatBasicBlockLabel( const ir_graph::BasicBlock& basic_block )
{
    if ( !basic_block.executionCount().has_value() )
    {
        return basic_block.label();
    }

    return basic_block.label() + "\\ncount=" + std::to_string( *basic_block.executionCount() );
}

double
computeBasicBlockHeatRatio( const ir_graph::BasicBlock& basic_block, std::uint64_t max_count )
{
    if ( !basic_block.executionCount().has_value() )
    {
        return 0.0;
    }

    const double numerator   = std::log1p( static_cast<double>( *basic_block.executionCount() ) );
    const double denominator = std::log1p( static_cast<double>( max_count ) );
    return denominator > 0.0 ? numerator / denominator : 0.0;
}

std::string
formatHexColor( const RgbColor& color )
{
    std::ostringstream stream;
    stream << '#' << std::hex << std::setfill( '0' ) << std::setw( 2 )
           << static_cast<unsigned>( color.red ) << std::setw( 2 )
           << static_cast<unsigned>( color.green ) << std::setw( 2 )
           << static_cast<unsigned>( color.blue );
    return stream.str();
}

std::string
interpolateBasicBlockFillColor( const ir_graph::BasicBlock& basic_block, std::uint64_t max_count )
{
    if ( !basic_block.executionCount().has_value() )
    {
        return std::string( kBasicBlockCluster.fill_color );
    }

    const double ratio = computeBasicBlockHeatRatio( basic_block, max_count );

    const auto lerp = [ratio]( std::uint8_t from, std::uint8_t to ) {
        return static_cast<std::uint8_t>(
            std::lround( static_cast<double>( from ) +
                         ( static_cast<double>( to ) - static_cast<double>( from ) ) * ratio ) );
    };

    return formatHexColor( RgbColor{ lerp( kColdBasicBlockColor.red, kHotBasicBlockColor.red ),
                                     lerp( kColdBasicBlockColor.green,
                                           kHotBasicBlockColor.green ),
                                     lerp( kColdBasicBlockColor.blue,
                                           kHotBasicBlockColor.blue ) } );
}

std::uint64_t
getMaxBasicBlockExecutionCount( const ir_graph::Graph& graph_model )
{
    std::uint64_t max_count = 0;

    for ( const auto& basic_block : graph_model.basicBlocks() )
    {
        if ( basic_block.executionCount().has_value() )
        {
            max_count = std::max( max_count, *basic_block.executionCount() );
        }
    }

    return max_count;
}

std::optional<std::string>
getRenderedEdgeLabel( const ir_graph::Edge& edge )
{
    const bool is_profiled_edge =
        edge.kind() == ir_graph::EdgeKind::ControlFlow || edge.kind() == ir_graph::EdgeKind::Call;
    if ( is_profiled_edge && edge.executionCount().has_value() )
    {
        return std::to_string( *edge.executionCount() );
    }

    if ( edge.label().has_value() && !edge.label()->empty() )
    {
        return edge.label();
    }

    return std::nullopt;
}

const EdgeStyle&
getEdgeStyle( ir_graph::EdgeKind kind )
{
    switch ( kind )
    {
        case ir_graph::EdgeKind::ControlFlow:
            return kControlFlowEdge;
        case ir_graph::EdgeKind::InstructionSequence:
            return kInstructionSequenceEdge;
        case ir_graph::EdgeKind::DataFlow:
            return kDataFlowEdge;
        case ir_graph::EdgeKind::Call:
            return kCallEdge;
    }

    return kDataFlowEdge;
}

const NodeStyle&
getNodeStyle( ir_graph::NodeKind kind )
{
    switch ( kind )
    {
        case ir_graph::NodeKind::Instruction:
            return kInstructionNode;
        case ir_graph::NodeKind::Operand:
            return kOperandNode;
    }

    return kInstructionNode;
}

} // namespace

std::string
renderDotGraph( const ir_graph::Graph& graph_model )
{
    dot_graph::Graph graph( graph_model.moduleName() );
    const auto       max_basic_block_execution_count = getMaxBasicBlockExecutionCount( graph_model );

    graph.graphAttributes()
        .setRaw( "pad", "0.3" )
        .setRaw( "rankdir", "TB" )
        .setBool( "compound", true )
        .setBool( "overlap", false )
        .setRaw( "nodesep", "0.9" )
        .setRaw( "ranksep", "1.3" )
        .setRaw( "splines", "true" );
    graph.nodeAttributes().setFontName( "SF Pro Text Bold 10" ).setFontSize( 15 );
    graph.edgeAttributes().setFontName( "SF Pro Text Bold 10" ).setFontSize( 15 );

    std::unordered_map<ir_graph::Id, std::vector<const ir_graph::BasicBlock*>> basic_blocks_by_function;
    std::unordered_map<ir_graph::Id, std::vector<const ir_graph::Node*>>       nodes_by_basic_block;
    std::vector<const ir_graph::Node*>                                         top_level_nodes;

    for ( const auto& basic_block : graph_model.basicBlocks() )
    {
        basic_blocks_by_function[basic_block.functionId()].push_back( &basic_block );
    }

    for ( const auto& node : graph_model.nodes() )
    {
        if ( node.basicBlockId().has_value() )
        {
            nodes_by_basic_block[*node.basicBlockId()].push_back( &node );
        } else
        {
            top_level_nodes.push_back( &node );
        }
    }

    for ( const auto& function : graph_model.functions() )
    {
        auto& function_cluster = graph.addSubgraph( getFunctionClusterId( function.id() ) );
        applyClusterStyle( function_cluster, kFunctionCluster );
        function_cluster.setQuotedLabel( formatFunctionLabel( function ) );

        const auto basic_blocks_it = basic_blocks_by_function.find( function.id() );
        if ( basic_blocks_it == basic_blocks_by_function.end() )
        {
            continue;
        }

        for ( const auto* basic_block : basic_blocks_it->second )
        {
            auto& basic_block_cluster = function_cluster.addSubgraph(
                getBasicBlockClusterId( basic_block->functionId(), basic_block->id() ) );
            applyClusterStyle( basic_block_cluster, kBasicBlockCluster );
            basic_block_cluster.setFillColor(
                interpolateBasicBlockFillColor( *basic_block, max_basic_block_execution_count ) );
            basic_block_cluster.setQuotedLabel( formatBasicBlockLabel( *basic_block ) );

            const auto nodes_it = nodes_by_basic_block.find( basic_block->id() );
            if ( nodes_it == nodes_by_basic_block.end() )
            {
                continue;
            }

            for ( const auto* node : nodes_it->second )
            {
                auto& dot_node = basic_block_cluster.addNode( getNodeRenderId( node->id() ) );
                applyNodeStyle( dot_node, getNodeStyle( node->kind() ) );
                dot_node.setQuotedLabel( node->label() );
            }
        }
    }

    for ( const auto* node : top_level_nodes )
    {
        auto& dot_node = graph.addNode( getNodeRenderId( node->id() ) );
        applyNodeStyle( dot_node, getNodeStyle( node->kind() ) );
        dot_node.setQuotedLabel( node->label() );
    }

    for ( const auto& edge : graph_model.edges() )
    {
        auto& dot_edge = graph.addEdge( getNodeRenderId( edge.sourceNodeId() ),
                                        getNodeRenderId( edge.targetNodeId() ) );
        applyEdgeStyle( dot_edge, getEdgeStyle( edge.kind() ) );

        if ( const auto rendered_label = getRenderedEdgeLabel( edge );
             rendered_label.has_value() && !rendered_label->empty() )
        {
            dot_edge.setQuotedLabel( *rendered_label );
        }
    }

    return static_cast<std::string>( graph );
}

} // namespace ir_graph
