#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm_ir_dumper/cl_opt_expansion.h"
#include "llvm_ir_dumper/dumper_pass.h"

/* When a plugin is loaded by the driver, it will call this entry point to
obtain information about this plugin and about how to register its passes.
*/
extern "C" {
LLVM_ATTRIBUTE_WEAK
llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
    const auto callback = []( llvm::PassBuilder& PB ) {
        PB.registerPipelineStartEPCallback( []( llvm::ModulePassManager& MPM, auto ) {
            MPM.addPass( llvm_ir_dumper::DumperPass{ DotOutBeforeOpt } );
            return true;
        } );
        PB.registerOptimizerLastEPCallback( []( llvm::ModulePassManager& MPM, auto ) {
            MPM.addPass( llvm_ir_dumper::DumperPass{ DotOutAfterOpt } );
            return true;
        } );
    };

    return { LLVM_PLUGIN_API_VERSION, "IRDumperPlugin", "0.0.1", callback };
}
}
