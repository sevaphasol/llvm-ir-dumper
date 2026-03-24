#include "llvm_ir_dumper/dumper_pass.h"

#include <system_error>

#include <llvm/Support/raw_ostream.h>

#include "ir_graph/ir_graph_builder.h"
#include "ir_graph/ir_graph_serde.h"

namespace llvm_ir_dumper {

DumperPass::DumperPass( const std::string& json_out, const std::string& ll_out )
    : json_out_( json_out ), ll_out_( ll_out )
{
}

llvm::PreservedAnalyses
DumperPass::run( llvm::Module& module, llvm::ModuleAnalysisManager& /*unused*/ )
{
    std::error_code error_code;

    llvm::raw_fd_ostream json_dump( json_out_, error_code );
    if ( error_code )
    {
        llvm::errs() << "[dumper-pass] unable to open json dump file (" << json_out_
                     << "): " << error_code.message() << "\n";
        return llvm::PreservedAnalyses::all();
    }

    llvm::raw_fd_ostream ll_dump( ll_out_, error_code );
    if ( error_code )
    {
        llvm::errs() << "[dumper-pass] unable to open ll dump file (" << ll_out_
                     << "): " << error_code.message() << "\n";
        return llvm::PreservedAnalyses::all();
    }

    dumpModule( module, ll_dump, json_dump );

    return llvm::PreservedAnalyses::all();
}

void
DumperPass::dumpModule( llvm::Module&         module,
                        llvm::raw_fd_ostream& ll_dump,
                        llvm::raw_fd_ostream& json_dump )
{
    module.print( ll_dump, nullptr );

    IrGraphBuilder builder;
    auto           graph = builder.build( module );

    json_dump << ir_graph::serialize( graph ) << '\n';
}

} // namespace llvm_ir_dumper
