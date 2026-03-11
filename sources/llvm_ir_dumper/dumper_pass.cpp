#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

#include <sstream>
#include <string>

#include "dot_graph/graph.h"
#include "llvm_ir_dumper/dumper_pass.h"

namespace llvm_ir_dumper {

DumperPass::DumperPass( const std::string& dot_out, const std::string& ll_out )
    : dot_out_( dot_out ), ll_out_( ll_out )
{
}

llvm::PreservedAnalyses
DumperPass::run( llvm::Module& module, llvm::ModuleAnalysisManager& /*unused*/ )
{
    std::error_code error_code;

    llvm::raw_fd_ostream dot_dump( dot_out_, error_code );
    if ( error_code )
    {
        llvm::errs() << "[dumper-pass] unable to open dot_dump file (" << dot_out_
                     << "): " << error_code.message() << "\n";
        return llvm::PreservedAnalyses::all();
    }

    llvm::raw_fd_ostream ll_dump( ll_out_, error_code );
    if ( error_code )
    {
        llvm::errs() << "[dumper-pass] unable to open ll_dump file (" << ll_out_
                     << "): " << error_code.message() << "\n";
        return llvm::PreservedAnalyses::all();
    }

    dumpModule( module, ll_dump, dot_dump );

    return llvm::PreservedAnalyses::all();
}

std::string
DumperPass::formatOperandLabel( const llvm::Value& value )
{
    std::string              label;
    llvm::raw_string_ostream os( label );
    value.printAsOperand( os );
    return os.str();
}

std::string
DumperPass::formatSlotLabel( int slot )
{
    return "%" + std::to_string( slot );
}

std::string
DumperPass::getNodeId( std::string_view prefix, const void* ptr )
{
    std::ostringstream os;
    os << prefix << ptr;
    return os.str();
}

std::string
DumperPass::getValueNodeId( const llvm::Value* value )
{
    return getNodeId( "n", value );
}

std::string
DumperPass::getUseNodeId( const llvm::Use* use )
{
    return getNodeId( "u", use );
}

std::string
DumperPass::getFunctionClusterId( std::size_t func_index )
{
    return "cluster_func_" + std::to_string( func_index );
}

std::string
DumperPass::getBasicBlockClusterId( std::size_t func_index, std::size_t basic_block_index )
{
    return "cluster_func_" + std::to_string( func_index ) + "_bb_" +
           std::to_string( basic_block_index );
}

void
DumperPass::dumpModule( llvm::Module&         module,
                        llvm::raw_fd_ostream& ll_dump,
                        llvm::raw_fd_ostream& dot_dump )
{
    module.print( ll_dump, nullptr );

    dot_graph::Graph graph( module.getName().str() );
    graph.graphAttributes()
        .setRaw( "pad", GraphInfo.pad )
        .setRaw( "rankdir", GraphInfo.rankdir )
        .setBool( "compound", GraphInfo.compound )
        .setBool( "overlap", GraphInfo.overlap )
        .setRaw( "nodesep", GraphInfo.nodesep )
        .setRaw( "ranksep", GraphInfo.ranksep )
        .setRaw( "splines", GraphInfo.splines );
    graph.nodeAttributes()
        .setFontName( GraphInfo.node_font_name )
        .setFontSize( GraphInfo.node_font_size );
    graph.edgeAttributes()
        .setFontName( GraphInfo.edge_font_name )
        .setFontSize( GraphInfo.edge_font_size );

    fillFuncsInfo( module );
    dumpFuncs( module, graph );

    dot_dump << static_cast<std::string>( graph );
}

void
DumperPass::fillFuncsInfo( llvm::Module& module )
{
    funcs_info_.clear();

    std::size_t func_index = 0;
    for ( auto& func : module )
    {
        ++func_index;

        FuncInfoEntry entry;
        entry.cluster_id = getFunctionClusterId( func_index );

        if ( !func.empty() && !func.getEntryBlock().empty() )
        {
            entry.anchor_id = getValueNodeId( &func.getEntryBlock().front() );
        }

        funcs_info_.emplace( &func, entry );
    }
}

void
DumperPass::dumpFuncs( llvm::Module& module, dot_graph::Graph& graph )
{
    llvm::ModuleSlotTracker slot_tracker( &module );

    std::size_t func_index = 0;
    for ( auto& func : module )
    {
        ++func_index;
        slot_tracker.incorporateFunction( func );
        dumpFunc( func, slot_tracker, graph, func_index );
    }
}

void
DumperPass::dumpFunc( llvm::Function&          func,
                      llvm::ModuleSlotTracker& slot_tracker,
                      dot_graph::Graph&        graph,
                      std::size_t              func_index )
{
    auto& function_subgraph = graph.addSubgraph( funcs_info_.at( &func ).cluster_id )
                                  .setShape( FunctionSubgraph.shape )
                                  .setStyle( FunctionSubgraph.style )
                                  .setFillColor( FunctionSubgraph.fill_color )
                                  .setColor( FunctionSubgraph.color )
                                  .setPenWidth( FunctionSubgraph.pen_width )
                                  .setFontSize( FunctionSubgraph.font_size )
                                  .setFontName( FunctionSubgraph.font_name )
                                  .setQuotedLabel( func.getName().str() );

    dumpBasicBlocks( func, slot_tracker, graph, function_subgraph, func_index );
    dumpControlFlowEdges( func, graph );
}

void
DumperPass::dumpBasicBlocks( llvm::Function&          func,
                             llvm::ModuleSlotTracker& slot_tracker,
                             dot_graph::Graph&        graph,
                             dot_graph::Subgraph&     function_subgraph,
                             std::size_t              func_index )
{
    std::size_t basic_block_index = 0;

    for ( auto& basic_block : func )
    {
        ++basic_block_index;

        auto& block_subgraph =
            function_subgraph
                .addSubgraph( getBasicBlockClusterId( func_index, basic_block_index ) )
                .setShape( BasicBlockSubgraph.shape )
                .setStyle( BasicBlockSubgraph.style )
                .setFillColor( BasicBlockSubgraph.fill_color )
                .setColor( BasicBlockSubgraph.color )
                .setPenWidth( BasicBlockSubgraph.pen_width )
                .setFontSize( BasicBlockSubgraph.font_size )
                .setFontName( BasicBlockSubgraph.font_name )
                .setQuotedLabel( formatOperandLabel( basic_block ) );

        dumpBasicBlock( basic_block, slot_tracker, graph, block_subgraph );
    }
}

void
DumperPass::dumpControlFlowEdges( llvm::Function& func, dot_graph::Graph& graph )
{
    for ( auto& basic_block : func )
    {
        auto first_non_phi_it = basic_block.getFirstNonPHIIt();

        first_non_phi_it == basic_block.begin()
            ? dumpControlFlowEdgesWithoutPhi( basic_block, graph )
            : dumpControlFlowEdgesWithPhi( basic_block, graph, first_non_phi_it );
    }
}

void
DumperPass::dumpControlFlowEdgesWithoutPhi( llvm::BasicBlock& basic_block,
                                            dot_graph::Graph& graph )
{
    auto* first_inst = &*basic_block.begin();

    for ( auto* pred : llvm::predecessors( &basic_block ) )
    {
        graph.addEdge( getValueNodeId( pred->getTerminator() ), getValueNodeId( first_inst ) )
            .setColor( ControlFlowEdge.color )
            .setWeight( ControlFlowEdge.weight )
            .setPenWidth( ControlFlowEdge.pen_width );
    }
}

void
DumperPass::dumpControlFlowEdgesWithPhi(
    llvm::BasicBlock&                         basic_block,
    dot_graph::Graph&                         graph,
    llvm::BranchInst::InstListType::iterator& first_non_phi_it )
{
    auto& first_phi = llvm::cast<llvm::PHINode>( *basic_block.begin() );

    for ( unsigned i = 0; i < first_phi.getNumIncomingValues(); ++i )
    {
        auto* predcessor = first_phi.getIncomingBlock( i )->getTerminator();
        graph.addEdge( getValueNodeId( predcessor ), getValueNodeId( &first_phi ) )
            .setColor( ControlFlowEdge.color )
            .setWeight( ControlFlowEdge.weight )
            .setPenWidth( ControlFlowEdge.pen_width );
    }

    for ( auto phi_it = std::next( basic_block.begin() ); phi_it != first_non_phi_it; ++phi_it )
    {
        auto& phi = llvm::cast<llvm::PHINode>( *phi_it );

        for ( unsigned i = 0; i < phi.getNumIncomingValues(); ++i )
        {
            auto* predcessor = phi.getIncomingBlock( i )->getTerminator();
            graph.addEdge( getValueNodeId( predcessor ), getValueNodeId( &phi ) )
                .setColor( DataFlowEdge.color )
                .setWeight( DataFlowEdge.weight )
                .setPenWidth( DataFlowEdge.pen_width )
                .setQuotedLabel( formatOperandLabel( *phi.getIncomingValue( i ) ) );
        }
    }
}

void
DumperPass::dumpBasicBlock( llvm::BasicBlock&        basic_block,
                            llvm::ModuleSlotTracker& slot_tracker,
                            dot_graph::Graph&        graph,
                            dot_graph::Subgraph&     block_subgraph )
{
    dumpInstrNodes( basic_block, block_subgraph );
    dumpInstrSeqEdges( basic_block, graph );
    dumpDataFlowEdges( basic_block, slot_tracker, graph, block_subgraph );
}

void
DumperPass::dumpInstrNodes( llvm::BasicBlock& basic_block, dot_graph::Subgraph& block_subgraph )
{
    for ( auto& instr : basic_block )
    {
        block_subgraph.addNode( getValueNodeId( &instr ) )
            .setShape( InstrNode.shape )
            .setStyle( InstrNode.style )
            .setFillColor( InstrNode.fill_color )
            .setColor( InstrNode.color )
            .setFontColor( InstrNode.font_color )
            .setQuotedLabel( instr.getOpcodeName() );
    }
}

void
DumperPass::dumpInstrSeqEdges( llvm::BasicBlock& basic_block, dot_graph::Graph& graph )
{
    for ( auto I_it = std::next( basic_block.begin() ); I_it != basic_block.end(); ++I_it )
    {
        auto prev_I_it = std::prev( I_it );

        graph.addEdge( getValueNodeId( &*prev_I_it ), getValueNodeId( &*I_it ) )
            .setColor( InstrSeqEdge.color )
            .setWeight( InstrSeqEdge.weight )
            .setPenWidth( InstrSeqEdge.pen_width );
    }
}

void
DumperPass::dumpDataFlowEdges( llvm::BasicBlock&        basic_block,
                               llvm::ModuleSlotTracker& slot_tracker,
                               dot_graph::Graph&        graph,
                               dot_graph::Subgraph&     block_subgraph )
{
    for ( auto I_it = basic_block.begin(); I_it != basic_block.end(); ++I_it )
    {
        auto* instr = &*I_it;

        bool processed_call = false;
        dumpInstrUsers( basic_block, instr, slot_tracker, graph, processed_call );
        dumpInstrOperands( instr, slot_tracker, graph, block_subgraph, processed_call );
    }
}

void
DumperPass::dumpInstrUsers( llvm::BasicBlock&        basic_block,
                            llvm::Instruction*       instr,
                            llvm::ModuleSlotTracker& slot_tracker,
                            dot_graph::Graph&        graph,
                            bool&                    processed_call )
{
    for ( auto& usage : instr->uses() )
    {
        auto* user = usage.getUser();
        auto& edge = graph.addEdge( getValueNodeId( instr ), getValueNodeId( user ) )
                         .setColor( DataFlowEdge.color )
                         .setWeight( DataFlowEdge.weight )
                         .setPenWidth( DataFlowEdge.pen_width );

        int slot = slot_tracker.getLocalSlot( instr );
        if ( slot != -1 )
        {
            edge.setQuotedLabel( formatSlotLabel( slot ) );
        }
    }

    if ( auto* call_base = llvm::dyn_cast<llvm::CallBase>( instr ) )
    {
        dumpCallInstr( call_base, graph, processed_call );
    }
}

void
DumperPass::dumpInstrOperands( llvm::Instruction*       instr,
                               llvm::ModuleSlotTracker& slot_tracker,
                               dot_graph::Graph&        graph,
                               dot_graph::Subgraph&     block_subgraph,
                               bool                     processed_call )
{
    if ( llvm::isa<llvm::PHINode>( instr ) )
    {
        return;
    }

    for ( auto& usage : instr->operands() )
    {
        auto* operand = usage.get();

        if ( llvm::isa<llvm::Instruction>( operand ) || llvm::isa<llvm::BasicBlock>( operand ) ||
             processed_call && llvm::isa<llvm::Function>( operand ) )
        {
            continue;
        }

        block_subgraph.addNode( getUseNodeId( &usage ) )
            .setShape( OperandNode.shape )
            .setStyle( OperandNode.style )
            .setFillColor( OperandNode.fill_color )
            .setColor( OperandNode.color )
            .setFontColor( OperandNode.font_color )
            .setQuotedLabel( formatOperandLabel( *operand ) );

        auto& edge = graph.addEdge( getUseNodeId( &usage ), getValueNodeId( instr ) )
                         .setColor( DataFlowEdge.color )
                         .setWeight( DataFlowEdge.weight )
                         .setPenWidth( DataFlowEdge.pen_width );

        int slot = slot_tracker.getLocalSlot( operand );
        if ( slot != -1 )
        {
            edge.setQuotedLabel( formatSlotLabel( slot ) );
        }
    }
}

void
DumperPass::dumpCallInstr( llvm::CallBase*   call_base,
                           dot_graph::Graph& graph,
                           bool&             processed_call )
{
    llvm::Function* callee = call_base->getCalledFunction();
    if ( callee == nullptr || callee->isDeclaration() )
    {
        return;
    }

    auto callee_entry_it = funcs_info_.find( callee );
    if ( callee_entry_it == funcs_info_.end() || !callee_entry_it->second.anchor_id.has_value() )
    {
        return;
    }

    processed_call = true;

    auto& call_edge =
        graph.addEdge( getValueNodeId( call_base ), *callee_entry_it->second.anchor_id )
            .setColor( ControlFlowEdge.color )
            .setPenWidth( ControlFlowEdge.pen_width )
            .setStyle( "dashed" )
            .setConstraint( false );
}

} // namespace llvm_ir_dumper
