#include "ir_graph/ir_graph.h"

#include <utility>

namespace ir_graph {

Function&
Function::id( Id value )
{
    id_ = value;
    return *this;
}

Function&
Function::name( std::string value )
{
    name_ = std::move( value );
    return *this;
}

Function&
Function::entryNodeId( std::optional<Id> value )
{
    entry_node_id_ = value;
    return *this;
}

Function&
Function::entryBasicBlockId( std::optional<Id> value )
{
    entry_basic_block_id_ = value;
    return *this;
}

Function&
Function::executionCount( std::optional<std::uint64_t> value )
{
    execution_count_ = value;
    return *this;
}

Id
Function::id() const
{
    return id_;
}

const std::string&
Function::name() const
{
    return name_;
}

const std::optional<Id>&
Function::entryNodeId() const
{
    return entry_node_id_;
}

const std::optional<Id>&
Function::entryBasicBlockId() const
{
    return entry_basic_block_id_;
}

const std::optional<std::uint64_t>&
Function::executionCount() const
{
    return execution_count_;
}

BasicBlock&
BasicBlock::id( Id value )
{
    id_ = value;
    return *this;
}

BasicBlock&
BasicBlock::functionId( Id value )
{
    function_id_ = value;
    return *this;
}

BasicBlock&
BasicBlock::label( std::string value )
{
    label_ = std::move( value );
    return *this;
}

BasicBlock&
BasicBlock::entryNodeId( std::optional<Id> value )
{
    entry_node_id_ = value;
    return *this;
}

BasicBlock&
BasicBlock::executionCount( std::optional<std::uint64_t> value )
{
    execution_count_ = value;
    return *this;
}

Id
BasicBlock::id() const
{
    return id_;
}

Id
BasicBlock::functionId() const
{
    return function_id_;
}

const std::string&
BasicBlock::label() const
{
    return label_;
}

const std::optional<Id>&
BasicBlock::entryNodeId() const
{
    return entry_node_id_;
}

const std::optional<std::uint64_t>&
BasicBlock::executionCount() const
{
    return execution_count_;
}

Node&
Node::id( Id value )
{
    id_ = value;
    return *this;
}

Node&
Node::kind( NodeKind value )
{
    kind_ = value;
    return *this;
}

Node&
Node::functionId( std::optional<Id> value )
{
    function_id_ = value;
    return *this;
}

Node&
Node::basicBlockId( std::optional<Id> value )
{
    basic_block_id_ = value;
    return *this;
}

Node&
Node::label( std::string value )
{
    label_ = std::move( value );
    return *this;
}

Node&
Node::opcodeName( std::optional<std::string> value )
{
    opcode_name_ = std::move( value );
    return *this;
}

Node&
Node::consumerInstructionId( std::optional<Id> value )
{
    consumer_instruction_id_ = value;
    return *this;
}

Node&
Node::operandIndex( std::optional<std::uint64_t> value )
{
    operand_index_ = value;
    return *this;
}

Id
Node::id() const
{
    return id_;
}

NodeKind
Node::kind() const
{
    return kind_;
}

const std::optional<Id>&
Node::functionId() const
{
    return function_id_;
}

const std::optional<Id>&
Node::basicBlockId() const
{
    return basic_block_id_;
}

const std::string&
Node::label() const
{
    return label_;
}

const std::optional<std::string>&
Node::opcodeName() const
{
    return opcode_name_;
}

const std::optional<Id>&
Node::consumerInstructionId() const
{
    return consumer_instruction_id_;
}

const std::optional<std::uint64_t>&
Node::operandIndex() const
{
    return operand_index_;
}

Edge&
Edge::id( Id value )
{
    id_ = value;
    return *this;
}

Edge&
Edge::kind( EdgeKind value )
{
    kind_ = value;
    return *this;
}

Edge&
Edge::sourceNodeId( Id value )
{
    source_node_id_ = value;
    return *this;
}

Edge&
Edge::targetNodeId( Id value )
{
    target_node_id_ = value;
    return *this;
}

Edge&
Edge::label( std::optional<std::string> value )
{
    label_ = std::move( value );
    return *this;
}

Edge&
Edge::sourceFunctionId( std::optional<Id> value )
{
    source_function_id_ = value;
    return *this;
}

Edge&
Edge::targetFunctionId( std::optional<Id> value )
{
    target_function_id_ = value;
    return *this;
}

Edge&
Edge::sourceBasicBlockId( std::optional<Id> value )
{
    source_basic_block_id_ = value;
    return *this;
}

Edge&
Edge::targetBasicBlockId( std::optional<Id> value )
{
    target_basic_block_id_ = value;
    return *this;
}

Edge&
Edge::successorIndex( std::optional<std::uint64_t> value )
{
    successor_index_ = value;
    return *this;
}

Edge&
Edge::executionCount( std::optional<std::uint64_t> value )
{
    execution_count_ = value;
    return *this;
}

Id
Edge::id() const
{
    return id_;
}

EdgeKind
Edge::kind() const
{
    return kind_;
}

Id
Edge::sourceNodeId() const
{
    return source_node_id_;
}

Id
Edge::targetNodeId() const
{
    return target_node_id_;
}

const std::optional<std::string>&
Edge::label() const
{
    return label_;
}

const std::optional<Id>&
Edge::sourceFunctionId() const
{
    return source_function_id_;
}

const std::optional<Id>&
Edge::targetFunctionId() const
{
    return target_function_id_;
}

const std::optional<Id>&
Edge::sourceBasicBlockId() const
{
    return source_basic_block_id_;
}

const std::optional<Id>&
Edge::targetBasicBlockId() const
{
    return target_basic_block_id_;
}

const std::optional<std::uint64_t>&
Edge::successorIndex() const
{
    return successor_index_;
}

const std::optional<std::uint64_t>&
Edge::executionCount() const
{
    return execution_count_;
}

Graph&
Graph::moduleName( std::string value )
{
    module_name_ = std::move( value );
    return *this;
}

const std::string&
Graph::moduleName() const
{
    return module_name_;
}

Function&
Graph::addFunction()
{
    functions_.emplace_back();
    return functions_.back();
}

BasicBlock&
Graph::addBasicBlock()
{
    basic_blocks_.emplace_back();
    return basic_blocks_.back();
}

Node&
Graph::addNode()
{
    nodes_.emplace_back();
    return nodes_.back();
}

Edge&
Graph::addEdge( Id source_node_id, Id target_node_id )
{
    edges_.emplace_back();
    return edges_.back().sourceNodeId( source_node_id ).targetNodeId( target_node_id );
}

Function&
Graph::function( Id id )
{
    return functions_.at( id );
}

const Function&
Graph::function( Id id ) const
{
    return functions_.at( id );
}

BasicBlock&
Graph::basicBlock( Id id )
{
    return basic_blocks_.at( id );
}

const BasicBlock&
Graph::basicBlock( Id id ) const
{
    return basic_blocks_.at( id );
}

Node&
Graph::node( Id id )
{
    return nodes_.at( id );
}

const Node&
Graph::node( Id id ) const
{
    return nodes_.at( id );
}

Edge&
Graph::edge( Id id )
{
    return edges_.at( id );
}

const Edge&
Graph::edge( Id id ) const
{
    return edges_.at( id );
}

const std::vector<Function>&
Graph::functions() const
{
    return functions_;
}

const std::vector<BasicBlock>&
Graph::basicBlocks() const
{
    return basic_blocks_;
}

const std::vector<Node>&
Graph::nodes() const
{
    return nodes_;
}

const std::vector<Edge>&
Graph::edges() const
{
    return edges_;
}

} // namespace ir_graph
