#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm_ir_dumper/cl_opt_expansion.h"
#include "llvm_ir_dumper/dump_pass.h"
#include "llvm_ir_dumper/inject_pass.h"

namespace {

bool
registerDumperPass( llvm::ModulePassManager& pass_manager,
                    const std::string&       json_out,
                    const std::string&       ir_out,
                    std::string_view         stage_name )
{
    if ( json_out.empty() || ir_out.empty() )
    {
        llvm::errs() << "[dumper-pass] " << stage_name
                     << " json/ir paths are empty. No dump will be produced for it.\n";
        return false;
    }

    pass_manager.addPass( llvm_ir_dump_pass::DumpPass{ json_out, ir_out } );
    return true;
}

} // namespace

extern "C" {
LLVM_ATTRIBUTE_WEAK
llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
    const auto callback = []( llvm::PassBuilder& pass_builder ) {
        pass_builder.registerPipelineStartEPCallback(
            []( llvm::ModulePassManager& pass_manager, auto ) {
                return registerDumperPass( pass_manager,
                                           JsonOutBeforeOpt,
                                           IrOutBeforeOpt,
                                           "before-opt" );
            } );
        pass_builder.registerOptimizerLastEPCallback( []( llvm::ModulePassManager& pass_manager,
                                                          auto ) {
            auto res =
                registerDumperPass( pass_manager, JsonOutAfterOpt, IrOutAfterOpt, "after-opt" );

            if ( EnableLoggingInjection )
            {
                pass_manager.addPass( llvm_ir_inject_pass::InjectPass{} );
            }

            return res;
        } );
    };

    return { LLVM_PLUGIN_API_VERSION, "IRDumperPlugin", "0.0.1", callback };
}
}
