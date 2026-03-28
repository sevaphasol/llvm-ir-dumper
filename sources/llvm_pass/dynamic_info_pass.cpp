#include "llvm_ir_dumper/inject_pass.h"

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "ir_graph/ir_graph.h"
#include "ir_graph/ir_graph_builder.h"

namespace llvm_ir_inject_pass {

namespace {

constexpr llvm::StringLiteral kFunctionEnterFormat   = "[ir-log] func_enter fid=%llu\n";
constexpr llvm::StringLiteral kBasicBlockEnterFormat = "[ir-log] bb_enter fid=%llu bbid=%llu\n";
constexpr llvm::StringLiteral kCallEdgeFormat =
    "[ir-log] call_edge eid=%llu src_fid=%llu src_bbid=%llu dst_fid=%llu call_node=%llu\n";
constexpr llvm::StringLiteral kCfgEdgeFormat =
    "[ir-log] cfg_edge eid=%llu fid=%llu src_bbid=%llu dst_bbid=%llu succ=%llu\n";

struct OriginalCfgEdge
{
    llvm::BasicBlock* source                = nullptr;
    llvm::BasicBlock* target                = nullptr;
    ir_graph::Id      edge_id               = 0;
    ir_graph::Id      source_function_id    = 0;
    ir_graph::Id      source_basic_block_id = 0;
    ir_graph::Id      target_basic_block_id = 0;
    std::uint64_t     successor_index       = 0;
};

bool
isDefinedFunction( const llvm::Function& function )
{
    return !function.isDeclaration();
}

llvm::Instruction*
getBasicBlockEntryInsertionPoint( llvm::BasicBlock& basic_block )
{
    auto insertion_it = basic_block.getFirstInsertionPt();
    if ( insertion_it != basic_block.end() )
    {
        return &*insertion_it;
    }

    return basic_block.getTerminator();
}

class LoggingEmitter {
  public:
    explicit LoggingEmitter( llvm::Module& module )
        : module_( module ),
          context_( module.getContext() ),
          printf_function_( getPrintfFunction( module ) ),
          function_enter_format_( createFormatString( kFunctionEnterFormat ) ),
          basic_block_enter_format_( createFormatString( kBasicBlockEnterFormat ) ),
          call_edge_format_( createFormatString( kCallEdgeFormat ) ),
          cfg_edge_format_( createFormatString( kCfgEdgeFormat ) )
    {
    }

    llvm::Value*
    i64( std::uint64_t value ) const
    {
        return llvm::ConstantInt::get( llvm::Type::getInt64Ty( context_ ), value );
    }

    void
    emitFunctionEnter( llvm::IRBuilder<>& builder, std::uint64_t function_id )
    {
        emit( builder, function_enter_format_, { i64( function_id ) } );
    }

    void
    emitBasicBlockEnter( llvm::IRBuilder<>& builder,
                         std::uint64_t      function_id,
                         std::uint64_t      basic_block_id )
    {
        emit( builder, basic_block_enter_format_, { i64( function_id ), i64( basic_block_id ) } );
    }

    void
    emitCallEdgeBefore( llvm::Instruction* insertion_point,
                        std::uint64_t      edge_id,
                        std::uint64_t      source_function_id,
                        std::uint64_t      source_basic_block_id,
                        std::uint64_t      target_function_id,
                        std::uint64_t      source_node_id )
    {
        emitBefore( insertion_point,
                    call_edge_format_,
                    { i64( edge_id ),
                      i64( source_function_id ),
                      i64( source_basic_block_id ),
                      i64( target_function_id ),
                      i64( source_node_id ) } );
    }

    void
    emitCfgEdgeBefore( llvm::Instruction* insertion_point,
                       std::uint64_t      edge_id,
                       std::uint64_t      source_function_id,
                       std::uint64_t      source_basic_block_id,
                       std::uint64_t      target_basic_block_id,
                       std::uint64_t      successor_index )
    {
        emitBefore( insertion_point,
                    cfg_edge_format_,
                    { i64( edge_id ),
                      i64( source_function_id ),
                      i64( source_basic_block_id ),
                      i64( target_basic_block_id ),
                      i64( successor_index ) } );
    }

  private:
    void
    emitBefore( llvm::Instruction*                  insertion_point,
                llvm::Constant*                     format,
                std::initializer_list<llvm::Value*> args )
    {
        if ( insertion_point == nullptr )
        {
            return;
        }

        llvm::IRBuilder<> builder( insertion_point );
        emit( builder, format, args );
    }

  private:
    static llvm::FunctionCallee
    getPrintfFunction( llvm::Module& module )
    {
        auto& context = module.getContext();
        auto* printf_type =
            llvm::FunctionType::get( llvm::Type::getInt32Ty( context ),
                                     llvm::ArrayRef<llvm::Type*>{ llvm::PointerType::getUnqual(
                                         llvm::Type::getInt8Ty( context ) ) },
                                     true );
        return module.getOrInsertFunction( "printf", printf_type );
    }

    llvm::Constant*
    createFormatString( llvm::StringRef format )
    {
        llvm::IRBuilder<> builder( context_ );
        return llvm::cast<llvm::Constant>(
            builder.CreateGlobalStringPtr( format, "", 0, &module_ ) );
    }

    void
    emit( llvm::IRBuilder<>&                  builder,
          llvm::Constant*                     format,
          std::initializer_list<llvm::Value*> args )
    {
        llvm::SmallVector<llvm::Value*, 6> printf_args;
        printf_args.reserve( args.size() + 1 );
        printf_args.push_back( format );
        printf_args.append( args.begin(), args.end() );
        builder.CreateCall( printf_function_, printf_args );
    }

  private:
    llvm::Module&        module_;
    llvm::LLVMContext&   context_;
    llvm::FunctionCallee printf_function_;
    llvm::Constant*      function_enter_format_    = nullptr;
    llvm::Constant*      basic_block_enter_format_ = nullptr;
    llvm::Constant*      call_edge_format_         = nullptr;
    llvm::Constant*      cfg_edge_format_          = nullptr;
};

bool
isSupportedControlFlowTerminator( const llvm::Instruction& instruction )
{
    return llvm::isa<llvm::BranchInst>( instruction ) ||
           llvm::isa<llvm::SwitchInst>( instruction );
}

std::vector<OriginalCfgEdge>
collectOriginalCfgEdges( llvm::Module& module, const ir_graph::BuildInfo& info )
{
    std::vector<OriginalCfgEdge> edges;

    for ( auto& function : module )
    {
        if ( !isDefinedFunction( function ) )
        {
            continue;
        }

        const auto function_id = info.function_ids.at( &function );

        for ( auto& basic_block : function )
        {
            auto* terminator = basic_block.getTerminator();
            if ( terminator == nullptr || !isSupportedControlFlowTerminator( *terminator ) )
            {
                continue;
            }

            const auto source_basic_block_id = info.basic_block_ids.at( &basic_block );

            for ( unsigned successor_index = 0; successor_index < terminator->getNumSuccessors();
                  ++successor_index )
            {
                auto* successor = terminator->getSuccessor( successor_index );
                if ( successor == nullptr )
                {
                    continue;
                }

                const auto target_basic_block_id = info.basic_block_ids.at( successor );
                const auto edge_it =
                    info.cfg_edge_ids.find( ir_graph::CfgEdgeKey{ source_basic_block_id,
                                                                  target_basic_block_id,
                                                                  successor_index } );
                if ( edge_it == info.cfg_edge_ids.end() )
                {
                    continue;
                }

                edges.push_back( OriginalCfgEdge{
                    .source                = &basic_block,
                    .target                = successor,
                    .edge_id               = edge_it->second,
                    .source_function_id    = function_id,
                    .source_basic_block_id = source_basic_block_id,
                    .target_basic_block_id = target_basic_block_id,
                    .successor_index       = successor_index,
                } );
            }
        }
    }

    return edges;
}

void
injectEntryLogs( llvm::Module& module, const ir_graph::BuildInfo& info, LoggingEmitter& logger )
{
    for ( auto& function : module )
    {
        if ( !isDefinedFunction( function ) )
        {
            continue;
        }

        const auto function_id = info.function_ids.at( &function );

        for ( auto& basic_block : function )
        {
            auto* insertion_point = getBasicBlockEntryInsertionPoint( basic_block );
            if ( insvalidate_config( config )

                     reset_tmp_dirertion_point == nullptr )
            {
                continue;
            }

            llvm::IRBuilder<> builder( insertion_point );
            if ( &basic_block == &function.getEntryBlock() )
            {
                logger.emitFunctionEnter( builder, function_id );
            }

            logger.emitBasicBlockEnter( builder,
                                        function_id,
                                        info.basic_block_ids.at( &basic_block ) );
        }
    }
}

void
injectCallEdgeLogs( llvm::Module&              module,
                    const ir_graph::BuildInfo& info,
                    LoggingEmitter&            logger )
{
    for ( auto& function : module )
    {
        if ( !isDefinedFunction( function ) )
        {
            continue;
        }

        const auto source_function_id = info.function_ids.at( &function );

        for ( auto& basic_block : function )
        {
            const auto source_basic_block_id = info.basic_block_ids.at( &basic_block );

            for ( auto& instruction : basic_block )
            {
                auto* call_base = llvm::dyn_cast<llvm::CallBase>( &instruction );
                if ( call_base == nullptr )
                {
                    continue;
                }

                auto* callee = call_base->getCalledFunction();
                if ( callee == nullptr || !isDefinedFunction( *callee ) )
                {
                    continue;
                }

                const auto function_it = info.function_ids.find( callee );
                if ( function_it == info.function_ids.end() )
                {
                    continue;
                }

                const auto source_node_id     = info.instruction_node_ids.at( &instruction );
                const auto target_function_id = function_it->second;
                const auto edge_it            = info.call_edge_ids.find(
                    ir_graph::CallEdgeKey{ source_node_id, target_function_id } );
                if ( edge_it == info.call_edge_ids.end() )
                {
                    continue;
                }

                logger.emitCallEdgeBefore( &instruction,
                                           edge_it->second,
                                           source_function_id,
                                           source_basic_block_id,
                                           target_function_id,
                                           source_node_id );
            }
        }
    }
}

void
injectControlFlowEdgeLogs( llvm::Module&              module,
                           const ir_graph::BuildInfo& info,
                           LoggingEmitter&            logger )
{
    const auto original_cfg_edges = collectOriginalCfgEdges( module, info );

    for ( const auto& edge : original_cfg_edges )
    {
        auto* logging_block = llvm::SplitEdge( edge.source, edge.target );
        if ( logging_block == nullptr || logging_block->getTerminator() == nullptr )
        {
            llvm::errs() << "[inject-pass] unable to split edge from bb "
                         << edge.source_basic_block_id << " to bb " << edge.target_basic_block_id
                         << '\n';
            continue;
        }

        logger.emitCfgEdgeBefore( logging_block->getTerminator(),
                                  edge.edge_id,
                                  edge.source_function_id,
                                  edge.source_basic_block_id,
                                  edge.target_basic_block_id,
                                  edge.successor_index );
    }
}

} // namespace

llvm::PreservedAnalyses
InjectPass::run( llvm::Module& module, llvm::ModuleAnalysisManager& /*unused*/ )
{
    injectLogging( module );

    return llvm::PreservedAnalyses::none();
}

void
InjectPass::injectLogging( llvm::Module& module )
{
    auto           build = ir_graph::IrGraphBuilder().build( module );
    const auto&    info  = build.info;
    LoggingEmitter logger( module );

    injectEntryLogs( module, info, logger );
    injectCallEdgeLogs( module, info, logger );
    injectControlFlowEdgeLogs( module, info, logger );
}

} // namespace llvm_ir_inject_pass
