#pragma once

#include <optional>
#include <unordered_map>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSlotTracker.h"

#include "ir_graph/ir_graph.h"

namespace ir_graph {

struct CfgEdgeKey
{
    ir_graph::Id  source_basic_block_id;
    ir_graph::Id  target_basic_block_id;
    std::uint64_t successor_index;

    bool
    operator==( const CfgEdgeKey& other ) const
    {
        return source_basic_block_id == other.source_basic_block_id &&
               target_basic_block_id == other.target_basic_block_id &&
               successor_index == other.successor_index;
    }
};

struct CfgEdgeKeyHash
{
    std::size_t
    operator()( const CfgEdgeKey& key ) const
    {
        return std::hash<ir_graph::Id>{}( key.source_basic_block_id ) ^
               ( std::hash<ir_graph::Id>{}( key.target_basic_block_id ) << 1U ) ^
               ( std::hash<std::uint64_t>{}( key.successor_index ) << 2U );
    }
};

struct CallEdgeKey
{
    ir_graph::Id source_node_id;
    ir_graph::Id target_function_id;

    bool
    operator==( const CallEdgeKey& other ) const
    {
        return source_node_id == other.source_node_id &&
               target_function_id == other.target_function_id;
    }
};

struct CallEdgeKeyHash
{
    std::size_t
    operator()( const CallEdgeKey& key ) const
    {
        return std::hash<ir_graph::Id>{}( key.source_node_id ) ^
               ( std::hash<ir_graph::Id>{}( key.target_function_id ) << 1U );
    }
};

struct BuildInfo
{
    std::unordered_map<const llvm::Function*, ir_graph::Id>        function_ids;
    std::unordered_map<const llvm::BasicBlock*, ir_graph::Id>      basic_block_ids;
    std::unordered_map<const llvm::Instruction*, ir_graph::Id>     instruction_node_ids;
    std::unordered_map<CfgEdgeKey, ir_graph::Id, CfgEdgeKeyHash>   cfg_edge_ids;
    std::unordered_map<CallEdgeKey, ir_graph::Id, CallEdgeKeyHash> call_edge_ids;
};

struct BuildResult
{
    ir_graph::Graph graph;
    ir_graph::BuildInfo info;
};

class IrGraphBuilder {
  private:
    ir_graph::BuildResult build_result_;

  public:
    ir_graph::BuildResult
    build( llvm::Module& module );

  private:
    void
    reset();

    void
    buildGraphHeader( llvm::Module& module );

    void
    registerFunctionsAndBasicBlocks( llvm::Module& module );

    void
    registerInstructionNodes( llvm::Module& module );

    void
    registerInstructionSequenceEdges( llvm::Module& module );

    void
    registerControlFlowEdges( llvm::Module& module );

    void
    registerControlFlowEdgesWithPhi( llvm::BasicBlock& basic_block,
                                     ir_graph::Id      function_id,
                                     ir_graph::Id      basic_block_id );

    void
    registerControlFlowEdgesWithoutPhi( llvm::BasicBlock& basic_block,
                                        ir_graph::Id      function_id,
                                        ir_graph::Id      basic_block_id );

    void
    registerDataFlowAndCallEdges( llvm::Module& module );

    void
    populateBuildInfo();

    void
    registerInstrUsersDataFlowEdges( llvm::Instruction&       instruction,
                                     llvm::ModuleSlotTracker& slot_tracker,
                                     std::uint64_t            instruction_node_id,
                                     std::uint64_t            function_id,
                                     std::uint64_t            basic_block_id,
                                     bool                     processed_call );

    void
    registerCallEdge( llvm::Instruction& instruction,
                      std::uint64_t      instruction_node_id,
                      std::uint64_t      function_id,
                      std::uint64_t      basic_block_id,
                      bool&              processed_call );

    void
    registerInstrOperandsDataFlowEdges( llvm::Instruction&       instruction,
                                        llvm::Function&          func,
                                        llvm::BasicBlock&        basic_block,
                                        llvm::ModuleSlotTracker& slot_tracker,
                                        std::uint64_t            instruction_node_id,
                                        std::uint64_t            function_id,
                                        std::uint64_t            basic_block_id,
                                        bool                     processed_call );

    void
    registerPhiNodeOperandsDataFlowEdges( llvm::Instruction& instr,
                                          std::uint64_t      function_id,
                                          std::uint64_t      basic_block_id );

    static std::string
    formatOperandLabel( const llvm::Value& value );

    static std::string
    formatSlotLabel( int slot );

    std::optional<std::uint64_t>
    findSuccessorIndex( const llvm::BasicBlock& source,
                        const llvm::BasicBlock& destination ) const;

    ir_graph::Id
    getFunctionId( const llvm::Function& func ) const;

    ir_graph::Id
    getBasicBlockId( const llvm::BasicBlock& basic_block ) const;

    ir_graph::Id
    getInstructionNodeId( const llvm::Instruction& instruction ) const;

    ir_graph::Node&
    addOperandNode( const llvm::Function&    func,
                    const llvm::BasicBlock&  basic_block,
                    const llvm::Instruction& consumer,
                    const llvm::Value&       operand,
                    std::uint64_t            operand_index );

    ir_graph::Edge&
    addEdge( ir_graph::EdgeKind kind, ir_graph::Id source_node_id, ir_graph::Id target_node_id );
};

} // namespace ir_graph
