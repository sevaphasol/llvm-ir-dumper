#include "ir_graph/ir_graph_serde.h"

#include <istream>
#include <string>

namespace ir_graph {

namespace {

template<typename TValue>
std::optional<TValue>
optionalValue( const nlohmann::json& json, std::string_view key )
{
    const auto it = json.find( std::string( key ) );
    if ( it == json.end() || it->is_null() )
    {
        return std::nullopt;
    }

    return it->template get<TValue>();
}

template<typename TValue>
void
setOptional( nlohmann::json& json, std::string_view key, const std::optional<TValue>& value )
{
    json[std::string( key )] =
        value.has_value() ? nlohmann::json( *value ) : nlohmann::json( nullptr );
}

} // namespace

void
to_json( nlohmann::json& json, const Function& function )
{
    json = {
        { "id", function.id() },
        { "name", function.name() },
    };
    setOptional( json, "entry_node_id", function.entryNodeId() );
    setOptional( json, "entry_basic_block_id", function.entryBasicBlockId() );
    setOptional( json, "execution_count", function.executionCount() );
}

void
from_json( const nlohmann::json& json, Function& function )
{
    function.id( json.value( "id", Id{ 0 } ) )
        .name( json.value( "name", std::string{} ) )
        .entryNodeId( optionalValue<Id>( json, "entry_node_id" ) )
        .entryBasicBlockId( optionalValue<Id>( json, "entry_basic_block_id" ) )
        .executionCount( optionalValue<std::uint64_t>( json, "execution_count" ) );
}

void
to_json( nlohmann::json& json, const BasicBlock& basic_block )
{
    json = {
        { "id", basic_block.id() },
        { "function_id", basic_block.functionId() },
        { "label", basic_block.label() },
    };
    setOptional( json, "entry_node_id", basic_block.entryNodeId() );
    setOptional( json, "execution_count", basic_block.executionCount() );
}

void
from_json( const nlohmann::json& json, BasicBlock& basic_block )
{
    basic_block.id( json.value( "id", Id{ 0 } ) )
        .functionId( json.value( "function_id", Id{ 0 } ) )
        .label( json.value( "label", std::string{} ) )
        .entryNodeId( optionalValue<Id>( json, "entry_node_id" ) )
        .executionCount( optionalValue<std::uint64_t>( json, "execution_count" ) );
}

void
to_json( nlohmann::json& json, const Node& node )
{
    json = {
        { "id", node.id() },
        { "kind", node.kind() },
        { "label", node.label() },
    };
    setOptional( json, "function_id", node.functionId() );
    setOptional( json, "basic_block_id", node.basicBlockId() );
    setOptional( json, "opcode_name", node.opcodeName() );
    setOptional( json, "consumer_instruction_id", node.consumerInstructionId() );
    setOptional( json, "operand_index", node.operandIndex() );
}

void
from_json( const nlohmann::json& json, Node& node )
{
    node.id( json.value( "id", Id{ 0 } ) )
        .kind( json.value( "kind", NodeKind::Instruction ) )
        .functionId( optionalValue<Id>( json, "function_id" ) )
        .basicBlockId( optionalValue<Id>( json, "basic_block_id" ) )
        .label( json.value( "label", std::string{} ) )
        .opcodeName( optionalValue<std::string>( json, "opcode_name" ) )
        .consumerInstructionId( optionalValue<Id>( json, "consumer_instruction_id" ) )
        .operandIndex( optionalValue<std::uint64_t>( json, "operand_index" ) );
}

void
to_json( nlohmann::json& json, const Edge& edge )
{
    json = {
        { "id", edge.id() },
        { "kind", edge.kind() },
        { "source_node_id", edge.sourceNodeId() },
        { "target_node_id", edge.targetNodeId() },
    };
    setOptional( json, "label", edge.label() );
    setOptional( json, "source_function_id", edge.sourceFunctionId() );
    setOptional( json, "target_function_id", edge.targetFunctionId() );
    setOptional( json, "source_basic_block_id", edge.sourceBasicBlockId() );
    setOptional( json, "target_basic_block_id", edge.targetBasicBlockId() );
    setOptional( json, "successor_index", edge.successorIndex() );
    setOptional( json, "execution_count", edge.executionCount() );
}

void
from_json( const nlohmann::json& json, Edge& edge )
{
    edge.id( json.value( "id", Id{ 0 } ) )
        .kind( json.value( "kind", EdgeKind::DataFlow ) )
        .sourceNodeId( json.value( "source_node_id", Id{ 0 } ) )
        .targetNodeId( json.value( "target_node_id", Id{ 0 } ) )
        .label( optionalValue<std::string>( json, "label" ) )
        .sourceFunctionId( optionalValue<Id>( json, "source_function_id" ) )
        .targetFunctionId( optionalValue<Id>( json, "target_function_id" ) )
        .sourceBasicBlockId( optionalValue<Id>( json, "source_basic_block_id" ) )
        .targetBasicBlockId( optionalValue<Id>( json, "target_basic_block_id" ) )
        .successorIndex( optionalValue<std::uint64_t>( json, "successor_index" ) )
        .executionCount( optionalValue<std::uint64_t>( json, "execution_count" ) );
}

void
to_json( nlohmann::json& json, const Graph& graph )
{
    json = {
        { "module_name", graph.moduleName() },
        { "functions", graph.functions() },
        { "basic_blocks", graph.basicBlocks() },
        { "nodes", graph.nodes() },
        { "edges", graph.edges() },
    };
}

void
from_json( const nlohmann::json& json, Graph& graph )
{
    graph.moduleName( json.value( "module_name", std::string{} ) );

    for ( const auto& function_json : json.value( "functions", nlohmann::json::array() ) )
    {
        graph.addFunction() = function_json.get<Function>();
    }

    for ( const auto& basic_block_json : json.value( "basic_blocks", nlohmann::json::array() ) )
    {
        graph.addBasicBlock() = basic_block_json.get<BasicBlock>();
    }

    for ( const auto& node_json : json.value( "nodes", nlohmann::json::array() ) )
    {
        graph.addNode() = node_json.get<Node>();
    }

    for ( const auto& edge_json : json.value( "edges", nlohmann::json::array() ) )
    {
        graph.addEdge() = edge_json.get<Edge>();
    }
}

std::string
serialize( const Graph& graph, int indent )
{
    return nlohmann::json( graph ).dump( indent );
}

Graph
deserialize( std::istream& input )
{
    return nlohmann::json::parse( input ).get<Graph>();
}

} // namespace ir_graph
