#include "ir_graph/ir_graph_builder.h"

#include <cassert>
#include <iterator>
#include <string>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

namespace ir_graph {

ir_graph::BuildResult
IrGraphBuilder::build( llvm::Module& module )
{
    reset();
    buildGraphHeader( module );
    registerFunctionsAndBasicBlocks( module );
    registerInstructionNodes( module );
    registerInstructionSequenceEdges( module );
    registerControlFlowEdges( module );
    registerDataFlowAndCallEdges( module );
    populateBuildInfo();
    return std::move( build_result_ );
}

void
IrGraphBuilder::reset()
{
    build_result_ = ir_graph::BuildResult{};
}

void
IrGraphBuilder::buildGraphHeader( llvm::Module& module )
{
    build_result_.graph.moduleName( module.getName().str() );
}

void
IrGraphBuilder::registerFunctionsAndBasicBlocks( llvm::Module& module )
{
    for ( auto& func : module )
    {
        const auto function_id =
            static_cast<ir_graph::Id>( build_result_.graph.functions().size() );
        build_result_.info.function_ids.emplace( &func, function_id );

        build_result_.graph.addFunction().id( function_id ).name( func.getName().str() );

        for ( auto& basic_block : func )
        {
            const auto basic_block_id =
                static_cast<ir_graph::Id>( build_result_.graph.basicBlocks().size() );
            build_result_.info.basic_block_ids.emplace( &basic_block, basic_block_id );

            build_result_.graph.addBasicBlock()
                .id( basic_block_id )
                .functionId( function_id )
                .label( formatOperandLabel( basic_block ) );

            auto& function = build_result_.graph.function( function_id );
            if ( !function.entryBasicBlockId().has_value() )
            {
                function.entryBasicBlockId( basic_block_id );
            }
        }
    }
}

void
IrGraphBuilder::registerInstructionNodes( llvm::Module& module )
{
    for ( auto& func : module )
    {
        const auto function_id = getFunctionId( func );

        for ( auto& basic_block : func )
        {
            const auto basic_block_id = getBasicBlockId( basic_block );

            for ( auto& instruction : basic_block )
            {
                const auto node_id =
                    static_cast<ir_graph::Id>( build_result_.graph.nodes().size() );
                build_result_.info.instruction_node_ids.emplace( &instruction, node_id );

                build_result_.graph.addNode()
                    .id( node_id )
                    .kind( ir_graph::NodeKind::Instruction )
                    .functionId( function_id )
                    .basicBlockId( basic_block_id )
                    .label( instruction.getOpcodeName() )
                    .opcodeName( std::string( instruction.getOpcodeName() ) );

                auto& function = build_result_.graph.function( function_id );
                if ( !function.entryNodeId().has_value() )
                {
                    function.entryNodeId( node_id );
                }

                auto& block = build_result_.graph.basicBlock( basic_block_id );
                if ( !block.entryNodeId().has_value() )
                {
                    block.entryNodeId( node_id );
                }
            }
        }
    }
}

void
IrGraphBuilder::registerInstructionSequenceEdges( llvm::Module& module )
{
    for ( auto& func : module )
    {
        for ( auto& basic_block : func )
        {
            for ( auto instruction_it = std::next( basic_block.begin() );
                  instruction_it != basic_block.end();
                  ++instruction_it )
            {
                auto prev_instruction_it = std::prev( instruction_it );

                addEdge( ir_graph::EdgeKind::InstructionSequence,
                         getInstructionNodeId( *prev_instruction_it ),
                         getInstructionNodeId( *instruction_it ) );
            }
        }
    }
}

void
IrGraphBuilder::registerControlFlowEdges( llvm::Module& module )
{
    for ( auto& func : module )
    {
        const auto function_id = getFunctionId( func );

        for ( auto& basic_block : func )
        {
            const auto basic_block_id   = getBasicBlockId( basic_block );
            auto       first_non_phi_it = basic_block.getFirstNonPHIIt();

            first_non_phi_it == basic_block.begin()
                ? registerControlFlowEdgesWithoutPhi( basic_block, function_id, basic_block_id )
                : registerControlFlowEdgesWithPhi( basic_block, function_id, basic_block_id );
        }
    }
}

void
IrGraphBuilder::registerControlFlowEdgesWithPhi( llvm::BasicBlock& basic_block,
                                                 ir_graph::Id      function_id,
                                                 ir_graph::Id      basic_block_id )
{
    auto& first_phi = llvm::cast<llvm::PHINode>( *basic_block.begin() );

    for ( unsigned incoming_index = 0; incoming_index < first_phi.getNumIncomingValues();
          ++incoming_index )
    {
        auto* predecessor = first_phi.getIncomingBlock( incoming_index );
        if ( predecessor == nullptr || predecessor->getTerminator() == nullptr )
        {
            continue;
        }

        addEdge( ir_graph::EdgeKind::ControlFlow,
                 getInstructionNodeId( *predecessor->getTerminator() ),
                 getInstructionNodeId( first_phi ) )
            .sourceFunctionId( function_id )
            .targetFunctionId( function_id )
            .sourceBasicBlockId( getBasicBlockId( *predecessor ) )
            .targetBasicBlockId( basic_block_id )
            .successorIndex( findSuccessorIndex( *predecessor, basic_block ) );
    }
}

void
IrGraphBuilder::registerControlFlowEdgesWithoutPhi( llvm::BasicBlock& basic_block,
                                                    ir_graph::Id      function_id,
                                                    ir_graph::Id      basic_block_id )
{
    auto* first_instruction = &*basic_block.begin();

    for ( auto* predecessor : llvm::predecessors( &basic_block ) )
    {
        auto* predecessor_terminator = predecessor->getTerminator();
        if ( predecessor_terminator == nullptr )
        {
            continue;
        }

        addEdge( ir_graph::EdgeKind::ControlFlow,
                 getInstructionNodeId( *predecessor_terminator ),
                 getInstructionNodeId( *first_instruction ) )
            .sourceFunctionId( function_id )
            .targetFunctionId( function_id )
            .sourceBasicBlockId( getBasicBlockId( *predecessor ) )
            .targetBasicBlockId( basic_block_id )
            .successorIndex( findSuccessorIndex( *predecessor, basic_block ) );
    }
}

void
IrGraphBuilder::registerDataFlowAndCallEdges( llvm::Module& module )
{
    llvm::ModuleSlotTracker slot_tracker( &module );

    for ( auto& func : module )
    {
        slot_tracker.incorporateFunction( func );
        const auto function_id = getFunctionId( func );

        for ( auto& basic_block : func )
        {
            const auto basic_block_id = getBasicBlockId( basic_block );

            for ( auto& instruction : basic_block )
            {
                const auto instruction_node_id = getInstructionNodeId( instruction );
                bool       processed_call      = false;

                registerInstrUsersDataFlowEdges( instruction,
                                                 slot_tracker,
                                                 instruction_node_id,
                                                 function_id,
                                                 basic_block_id,
                                                 processed_call );
                registerInstrOperandsDataFlowEdges( instruction,
                                                    func,
                                                    basic_block,
                                                    slot_tracker,
                                                    instruction_node_id,
                                                    function_id,
                                                    basic_block_id,
                                                    processed_call );
            }
        }
    }
}

void
IrGraphBuilder::populateBuildInfo()
{
    build_result_.info.cfg_edge_ids.clear();
    build_result_.info.call_edge_ids.clear();

    for ( const auto& edge : build_result_.graph.edges() )
    {
        if ( edge.kind() == ir_graph::EdgeKind::ControlFlow &&
             edge.sourceBasicBlockId().has_value() &&
             edge.targetBasicBlockId().has_value() && edge.successorIndex().has_value() )
        {
            build_result_.info.cfg_edge_ids.emplace(
                ir_graph::CfgEdgeKey{ *edge.sourceBasicBlockId(),
                                      *edge.targetBasicBlockId(),
                                      *edge.successorIndex() },
                edge.id() );
        }

        if ( edge.kind() == ir_graph::EdgeKind::Call && edge.targetFunctionId().has_value() )
        {
            build_result_.info.call_edge_ids.emplace(
                ir_graph::CallEdgeKey{ edge.sourceNodeId(), *edge.targetFunctionId() },
                edge.id() );
        }
    }
}

void
IrGraphBuilder::registerInstrUsersDataFlowEdges( llvm::Instruction&       instruction,
                                                 llvm::ModuleSlotTracker& slot_tracker,
                                                 std::uint64_t            instruction_node_id,
                                                 std::uint64_t            function_id,
                                                 std::uint64_t            basic_block_id,
                                                 bool                     processed_call )
{
    for ( auto& usage : instruction.uses() )
    {
        auto* user_instruction = llvm::dyn_cast<llvm::Instruction>( usage.getUser() );
        if ( user_instruction == nullptr )
        {
            continue;
        }

        auto& edge = addEdge( ir_graph::EdgeKind::DataFlow,
                              instruction_node_id,
                              getInstructionNodeId( *user_instruction ) );

        const int slot = slot_tracker.getLocalSlot( &instruction );
        if ( slot != -1 )
        {
            edge.label( formatSlotLabel( slot ) );
        }
    }

    if ( llvm::isa<llvm::CallBase>( instruction ) )
    {
        registerCallEdge( instruction,
                          instruction_node_id,
                          function_id,
                          basic_block_id,
                          processed_call );
    }
}

void
IrGraphBuilder::registerCallEdge( llvm::Instruction& instruction,
                                  std::uint64_t      instruction_node_id,
                                  std::uint64_t      function_id,
                                  std::uint64_t      basic_block_id,
                                  bool&              processed_call )
{
    auto* call_base = llvm::dyn_cast<llvm::CallBase>( &instruction );
    assert( call_base );

    auto* callee = call_base->getCalledFunction();
    if ( callee != nullptr && !callee->isDeclaration() )
    {
        const auto callee_it = build_result_.info.function_ids.find( callee );
        if ( callee_it != build_result_.info.function_ids.end() )
        {
            const auto& callee_function = build_result_.graph.function( callee_it->second );
            if ( callee_function.entryNodeId().has_value() )
            {
                addEdge( ir_graph::EdgeKind::Call,
                         instruction_node_id,
                         *callee_function.entryNodeId() )
                    .sourceFunctionId( function_id )
                    .targetFunctionId( callee_it->second )
                    .sourceBasicBlockId( basic_block_id )
                    .targetBasicBlockId( callee_function.entryBasicBlockId() );
                processed_call = true;
            }
        }
    }
}

void
IrGraphBuilder::registerInstrOperandsDataFlowEdges( llvm::Instruction&       instruction,
                                                    llvm::Function&          func,
                                                    llvm::BasicBlock&        basic_block,
                                                    llvm::ModuleSlotTracker& slot_tracker,
                                                    std::uint64_t            instruction_node_id,
                                                    std::uint64_t            function_id,
                                                    std::uint64_t            basic_block_id,
                                                    bool                     processed_call )
{
    if ( llvm::isa<llvm::PHINode>( instruction ) )
    {
        return registerPhiNodeOperandsDataFlowEdges( instruction, function_id, basic_block_id );
    }

    std::uint64_t operand_index = 0;
    for ( auto& usage : instruction.operands() )
    {
        auto* operand = usage.get();

        const bool is_instruction_operand = llvm::isa<llvm::Instruction>( operand );
        const bool is_basic_block_operand = llvm::isa<llvm::BasicBlock>( operand );
        const bool is_hidden_call_target = processed_call && llvm::isa<llvm::Function>( operand );

        if ( is_instruction_operand || is_basic_block_operand || is_hidden_call_target )
        {
            ++operand_index;
            continue;
        }

        auto& operand_node =
            addOperandNode( func, basic_block, instruction, *operand, operand_index );

        auto& edge =
            addEdge( ir_graph::EdgeKind::DataFlow, operand_node.id(), instruction_node_id );

        const int slot = slot_tracker.getLocalSlot( operand );
        if ( slot != -1 )
        {
            edge.label( formatSlotLabel( slot ) );
        }

        ++operand_index;
    }
}

void
IrGraphBuilder::registerPhiNodeOperandsDataFlowEdges( llvm::Instruction& instr,
                                                      std::uint64_t      function_id,
                                                      std::uint64_t      basic_block_id )
{
    auto& phi = llvm::cast<llvm::PHINode>( instr );

    for ( unsigned incoming_index = 0; incoming_index < phi.getNumIncomingValues();
          ++incoming_index )
    {
        auto* predecessor = phi.getIncomingBlock( incoming_index );
        if ( predecessor == nullptr || predecessor->getTerminator() == nullptr )
        {
            continue;
        }

        addEdge( ir_graph::EdgeKind::DataFlow,
                 getInstructionNodeId( *predecessor->getTerminator() ),
                 getInstructionNodeId( phi ) )
            .label( formatOperandLabel( *phi.getIncomingValue( incoming_index ) ) )
            .sourceFunctionId( function_id )
            .targetFunctionId( function_id )
            .sourceBasicBlockId( getBasicBlockId( *predecessor ) )
            .targetBasicBlockId( basic_block_id );
    }
}

std::string
IrGraphBuilder::formatOperandLabel( const llvm::Value& value )
{
    std::string              label;
    llvm::raw_string_ostream os( label );
    value.printAsOperand( os );
    return os.str();
}

std::string
IrGraphBuilder::formatSlotLabel( int slot )
{
    return "%" + std::to_string( slot );
}

std::optional<std::uint64_t>
IrGraphBuilder::findSuccessorIndex( const llvm::BasicBlock& source,
                                    const llvm::BasicBlock& destination ) const
{
    auto* terminator = source.getTerminator();
    if ( terminator == nullptr )
    {
        return std::nullopt;
    }

    for ( unsigned successor_index = 0; successor_index < terminator->getNumSuccessors();
          ++successor_index )
    {
        if ( terminator->getSuccessor( successor_index ) == &destination )
        {
            return successor_index;
        }
    }

    return std::nullopt;
}

ir_graph::Id
IrGraphBuilder::getFunctionId( const llvm::Function& func ) const
{
    const auto it = build_result_.info.function_ids.find( &func );
    assert( it != build_result_.info.function_ids.end() );
    return it->second;
}

ir_graph::Id
IrGraphBuilder::getBasicBlockId( const llvm::BasicBlock& basic_block ) const
{
    const auto it = build_result_.info.basic_block_ids.find( &basic_block );
    assert( it != build_result_.info.basic_block_ids.end() );
    return it->second;
}

ir_graph::Id
IrGraphBuilder::getInstructionNodeId( const llvm::Instruction& instruction ) const
{
    const auto it = build_result_.info.instruction_node_ids.find( &instruction );
    assert( it != build_result_.info.instruction_node_ids.end() );
    return it->second;
}

ir_graph::Node&
IrGraphBuilder::addOperandNode( const llvm::Function&    func,
                                const llvm::BasicBlock&  basic_block,
                                const llvm::Instruction& consumer,
                                const llvm::Value&       operand,
                                std::uint64_t            operand_index )
{
    const auto node_id = static_cast<ir_graph::Id>( build_result_.graph.nodes().size() );

    return build_result_.graph.addNode()
        .id( node_id )
        .kind( ir_graph::NodeKind::Operand )
        .functionId( getFunctionId( func ) )
        .basicBlockId( getBasicBlockId( basic_block ) )
        .label( formatOperandLabel( operand ) )
        .consumerInstructionId( getInstructionNodeId( consumer ) )
        .operandIndex( operand_index );
}

ir_graph::Edge&
IrGraphBuilder::addEdge( ir_graph::EdgeKind kind,
                         ir_graph::Id       source_node_id,
                         ir_graph::Id       target_node_id )
{
    const auto edge_id = static_cast<ir_graph::Id>( build_result_.graph.edges().size() );

    return build_result_.graph.addEdge( source_node_id, target_node_id )
        .id( edge_id )
        .kind( kind );
}

} // namespace ir_graph
