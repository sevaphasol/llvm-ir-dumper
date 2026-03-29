#pragma once

#include "ir_graph/ir_graph.h"
#include "ir_graph/ir_graph_builder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm_ir_inject_pass {

class LoggingEmitter {
  private:
    static constexpr llvm::StringLiteral kFunctionEnterFormat = "[ir-log] func_enter fid=%llu\n";
    static constexpr llvm::StringLiteral kBasicBlockEnterFormat =
        "[ir-log] bb_enter fid=%llu bbid=%llu\n";
    static constexpr llvm::StringLiteral kCallEdgeFormat =
        "[ir-log] call_edge eid=%llu src_fid=%llu src_bbid=%llu dst_fid=%llu call_node=%llu\n";
    static constexpr llvm::StringLiteral kCfgEdgeFormat =
        "[ir-log] cfg_edge eid=%llu fid=%llu src_bbid=%llu dst_bbid=%llu succ=%llu\n";

    llvm::Module&        module_;
    llvm::LLVMContext&   context_;
    llvm::FunctionCallee printf_function_;
    llvm::Constant*      function_enter_format_    = nullptr;
    llvm::Constant*      basic_block_enter_format_ = nullptr;
    llvm::Constant*      call_edge_format_         = nullptr;
    llvm::Constant*      cfg_edge_format_          = nullptr;

  public:
    explicit LoggingEmitter( llvm::Module& module );

    llvm::Value*
    i64( std::uint64_t value ) const;

    void
    emitFunctionEnter( llvm::IRBuilder<>& builder, std::uint64_t function_id );

    void
    emitBasicBlockEnter( llvm::IRBuilder<>& builder,
                         std::uint64_t      function_id,
                         std::uint64_t      basic_block_id );

    void
    emitCallEdgeBefore( llvm::Instruction* insertion_point,
                        std::uint64_t      edge_id,
                        std::uint64_t      source_function_id,
                        std::uint64_t      source_basic_block_id,
                        std::uint64_t      target_function_id,
                        std::uint64_t      source_node_id );

    void
    emitCfgEdgeBefore( llvm::Instruction* insertion_point,
                       std::uint64_t      edge_id,
                       std::uint64_t      source_function_id,
                       std::uint64_t      source_basic_block_id,
                       std::uint64_t      target_basic_block_id,
                       std::uint64_t      successor_index );

  private:
    void
    emitBefore( llvm::Instruction*                  insertion_point,
                llvm::Constant*                     format,
                std::initializer_list<llvm::Value*> args );

    static llvm::FunctionCallee
    getPrintfFunction( llvm::Module& module );

    llvm::Constant*
    createFormatString( llvm::StringRef format );

    void
    emit( llvm::IRBuilder<>&                  builder,
          llvm::Constant*                     format,
          std::initializer_list<llvm::Value*> args );
};

class InjectPass : public llvm::PassInfoMixin<InjectPass> {

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

  public:
    llvm::PreservedAnalyses
    run( llvm::Module& module, llvm::ModuleAnalysisManager& /*unused*/ );

  private:
    void
    injectLogging( llvm::Module& module );

    static bool
    isDefinedFunction( const llvm::Function& function );

    static llvm::Instruction*
    getBasicBlockEntryInsertionPoint( llvm::BasicBlock& basic_block );

    static bool
    isSupportedControlFlowTerminator( const llvm::Instruction& instruction );

    static std::vector<OriginalCfgEdge>
    collectOriginalCfgEdges( llvm::Module& module, const ir_graph::BuildInfo& info );

    static void
    injectEntryLogs( llvm::Module&              module,
                     const ir_graph::BuildInfo& info,
                     LoggingEmitter&            logger );

    static void
    injectCallEdgeLogs( llvm::Module&              module,
                        const ir_graph::BuildInfo& info,
                        LoggingEmitter&            logger );

    static void
    injectControlFlowEdgeLogs( llvm::Module&              module,
                               const ir_graph::BuildInfo& info,
                               LoggingEmitter&            logger );
};

} // namespace llvm_ir_inject_pass
