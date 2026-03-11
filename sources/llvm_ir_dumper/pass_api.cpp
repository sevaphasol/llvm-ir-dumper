#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm_ir_dumper/cl_opt_expansion.h"
#include "llvm_ir_dumper/dumper_pass.h"

namespace {

bool
registerDumperPass( llvm::ModulePassManager& pass_manager,
                    const std::string&       dot_out,
                    const std::string&       ir_out,
                    std::string_view         stage_name )
{
    if ( dot_out.empty() || ir_out.empty() )
    {
        llvm::errs() << "[dumper-pass] " << stage_name
                     << " paths are empty. No dump will be produced for it.\n";
        return false;
    }

    pass_manager.addPass( llvm_ir_dumper::DumperPass{ dot_out, ir_out } );
    return true;
}

} // namespace

extern "C" {
LLVM_ATTRIBUTE_WEAK
llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
    const auto callback = []( llvm::PassBuilder& PB ) {
        PB.registerPipelineStartEPCallback( []( llvm::ModulePassManager& pass_manager, auto ) {
            return registerDumperPass( pass_manager,
                                       DotOutBeforeOpt,
                                       IrOutBeforeOpt,
                                       "before-opt" );
        } );
        PB.registerOptimizerLastEPCallback( []( llvm::ModulePassManager& pass_manager, auto ) {
            return registerDumperPass( pass_manager, DotOutAfterOpt, IrOutAfterOpt, "after-opt" );
        } );
    };

    return { LLVM_PLUGIN_API_VERSION, "IRDumperPlugin", "0.0.1", callback };
}
}
