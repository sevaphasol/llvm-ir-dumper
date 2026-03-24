#include "ir_graph/ir_graph_to_dot.h"

#include <string>
#include <unordered_map>

#include "dot_graph/dot_graph.h"

namespace llvm_ir_dumper {

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

    std::unordered_map<ir_graph::Id, dot_graph::Subgraph*> function_clusters;
    std::unordered_map<ir_graph::Id, dot_graph::Subgraph*> basic_block_clusters;

    for ( const auto& function : graph_model.functions() )
    {
        auto& cluster = graph.addSubgraph( getFunctionClusterId( function.id() ) );
        applyClusterStyle( cluster, kFunctionCluster );
        cluster.setQuotedLabel( function.name() );
        function_clusters.emplace( function.id(), &cluster );
    }

    for ( const auto& basic_block : graph_model.basicBlocks() )
    {
        auto& cluster = function_clusters.at( basic_block.functionId() )
                            ->addSubgraph( getBasicBlockClusterId( basic_block.functionId(),
                                                                   basic_block.id() ) );
        applyClusterStyle( cluster, kBasicBlockCluster );
        cluster.setQuotedLabel( basic_block.label() );
        basic_block_clusters.emplace( basic_block.id(), &cluster );
    }

    for ( const auto& node : graph_model.nodes() )
    {
        dot_graph::Node* dot_node = nullptr;

        if ( node.basicBlockId().has_value() )
        {
            dot_node = &basic_block_clusters.at( *node.basicBlockId() )
                            ->addNode( getNodeRenderId( node.id() ) );
        } else
        {
            dot_node = &graph.addNode( getNodeRenderId( node.id() ) );
        }

        applyNodeStyle( *dot_node, getNodeStyle( node.kind() ) );
        dot_node->setQuotedLabel( node.label() );
    }

    for ( const auto& edge : graph_model.edges() )
    {
        auto& dot_edge = graph.addEdge( getNodeRenderId( edge.sourceNodeId() ),
                                        getNodeRenderId( edge.targetNodeId() ) );
        applyEdgeStyle( dot_edge, getEdgeStyle( edge.kind() ) );

        if ( edge.label().has_value() && !edge.label()->empty() )
        {
            dot_edge.setQuotedLabel( *edge.label() );
        }
    }

    return static_cast<std::string>( graph );
}

} // namespace llvm_ir_dumper
