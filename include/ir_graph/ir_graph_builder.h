#pragma once

#include <optional>
#include <unordered_map>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSlotTracker.h"

#include "ir_graph/ir_graph.h"

namespace llvm_ir_dumper {

class IrGraphBuilder {
  private:
    ir_graph::Graph graph_;

    std::unordered_map<const llvm::Function*, ir_graph::Id>    function_ids_;
    std::unordered_map<const llvm::BasicBlock*, ir_graph::Id>  basic_block_ids_;
    std::unordered_map<const llvm::Instruction*, ir_graph::Id> instruction_node_ids_;

  public:
    ir_graph::Graph
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

} // namespace llvm_ir_dumper
