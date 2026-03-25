#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace ir_graph {

using Id = std::uint64_t;

enum class NodeKind {
    Instruction,
    Operand,
};

enum class EdgeKind {
    ControlFlow,
    InstructionSequence,
    DataFlow,
    Call,
};

class Function {
  private:
    Id                id_ = 0;
    std::string       name_;
    std::optional<Id> entry_node_id_;
    std::optional<Id> entry_basic_block_id_;
    std::optional<std::uint64_t> execution_count_;

  public:
    Function&
    id( Id value );
    Function&
    name( std::string value );
    Function&
    entryNodeId( std::optional<Id> value );
    Function&
    entryBasicBlockId( std::optional<Id> value );
    Function&
    executionCount( std::optional<std::uint64_t> value );

    Id
    id() const;

    const std::string&
    name() const;

    const std::optional<Id>&
    entryNodeId() const;

    const std::optional<Id>&
    entryBasicBlockId() const;

    const std::optional<std::uint64_t>&
    executionCount() const;
};

class BasicBlock {
  private:
    Id                id_          = 0;
    Id                function_id_ = 0;
    std::string       label_;
    std::optional<Id> entry_node_id_;
    std::optional<std::uint64_t> execution_count_;

  public:
    BasicBlock&
    id( Id value );
    BasicBlock&
    functionId( Id value );
    BasicBlock&
    label( std::string value );
    BasicBlock&
    entryNodeId( std::optional<Id> value );
    BasicBlock&
    executionCount( std::optional<std::uint64_t> value );

    Id
    id() const;

    Id
    functionId() const;

    const std::string&
    label() const;

    const std::optional<Id>&
    entryNodeId() const;

    const std::optional<std::uint64_t>&
    executionCount() const;
};

class Node {
  private:
    Id                           id_   = 0;
    NodeKind                     kind_ = NodeKind::Instruction;
    std::optional<Id>            function_id_;
    std::optional<Id>            basic_block_id_;
    std::string                  label_;
    std::optional<std::string>   opcode_name_;
    std::optional<Id>            consumer_instruction_id_;
    std::optional<std::uint64_t> operand_index_;

  public:
    Node&
    id( Id value );
    Node&
    kind( NodeKind value );
    Node&
    functionId( std::optional<Id> value );
    Node&
    basicBlockId( std::optional<Id> value );
    Node&
    label( std::string value );
    Node&
    opcodeName( std::optional<std::string> value );
    Node&
    consumerInstructionId( std::optional<Id> value );
    Node&
    operandIndex( std::optional<std::uint64_t> value );

    Id
    id() const;

    NodeKind
    kind() const;

    const std::optional<Id>&
    functionId() const;

    const std::optional<Id>&
    basicBlockId() const;

    const std::string&
    label() const;

    const std::optional<std::string>&
    opcodeName() const;

    const std::optional<Id>&
    consumerInstructionId() const;

    const std::optional<std::uint64_t>&
    operandIndex() const;
};

class Edge {
  private:
    Id                           id_             = 0;
    EdgeKind                     kind_           = EdgeKind::DataFlow;
    Id                           source_node_id_ = 0;
    Id                           target_node_id_ = 0;
    std::optional<std::string>   label_;
    std::optional<Id>            source_function_id_;
    std::optional<Id>            target_function_id_;
    std::optional<Id>            source_basic_block_id_;
    std::optional<Id>            target_basic_block_id_;
    std::optional<std::uint64_t> successor_index_;
    std::optional<std::uint64_t> execution_count_;

  public:
    Edge&
    id( Id value );
    Edge&
    kind( EdgeKind value );
    Edge&
    sourceNodeId( Id value );
    Edge&
    targetNodeId( Id value );
    Edge&
    label( std::optional<std::string> value );
    Edge&
    sourceFunctionId( std::optional<Id> value );
    Edge&
    targetFunctionId( std::optional<Id> value );
    Edge&
    sourceBasicBlockId( std::optional<Id> value );
    Edge&
    targetBasicBlockId( std::optional<Id> value );
    Edge&
    successorIndex( std::optional<std::uint64_t> value );
    Edge&
    executionCount( std::optional<std::uint64_t> value );

    Id
    id() const;

    EdgeKind
    kind() const;

    Id
    sourceNodeId() const;

    Id
    targetNodeId() const;

    const std::optional<std::string>&
    label() const;

    const std::optional<Id>&
    sourceFunctionId() const;

    const std::optional<Id>&
    targetFunctionId() const;

    const std::optional<Id>&
    sourceBasicBlockId() const;

    const std::optional<Id>&
    targetBasicBlockId() const;

    const std::optional<std::uint64_t>&
    successorIndex() const;

    const std::optional<std::uint64_t>&
    executionCount() const;
};

class Graph {
  private:
    std::string             module_name_;
    std::vector<Function>   functions_;
    std::vector<BasicBlock> basic_blocks_;
    std::vector<Node>       nodes_;
    std::vector<Edge>       edges_;

  public:
    Graph&
    moduleName( std::string value );

    const std::string&
    moduleName() const;

    Function&
    addFunction();

    BasicBlock&
    addBasicBlock();

    Node&
    addNode();

    Edge&
    addEdge( Id source_node_id = 0, Id target_node_id = 0 );

    Function&
    function( Id id );

    const Function&
    function( Id id ) const;

    BasicBlock&
    basicBlock( Id id );

    const BasicBlock&
    basicBlock( Id id ) const;

    Node&
    node( Id id );

    const Node&
    node( Id id ) const;

    Edge&
    edge( Id id );

    const Edge&
    edge( Id id ) const;

    const std::vector<Function>&
    functions() const;

    const std::vector<BasicBlock>&
    basicBlocks() const;

    const std::vector<Node>&
    nodes() const;

    const std::vector<Edge>&
    edges() const;
};

NLOHMANN_JSON_SERIALIZE_ENUM( NodeKind,
                              {
                                  { NodeKind::Instruction, "instruction" },
                                  { NodeKind::Operand, "operand" },
                              } )

NLOHMANN_JSON_SERIALIZE_ENUM( EdgeKind,
                              {
                                  { EdgeKind::ControlFlow, "control_flow" },
                                  { EdgeKind::InstructionSequence, "instruction_sequence" },
                                  { EdgeKind::DataFlow, "data_flow" },
                                  { EdgeKind::Call, "call" },
                              } )

} // namespace ir_graph
