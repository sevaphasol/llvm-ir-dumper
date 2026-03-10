#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm_ir_dumper {

struct DumperPass : public llvm::PassInfoMixin<DumperPass>
{
    const std::string_view dot_out;

    DumperPass( const std::string& dot_out_ ) : dot_out( dot_out_ ) {}

    llvm::PreservedAnalyses
    run( llvm::Module& M, llvm::ModuleAnalysisManager& AM );
};

} // namespace llvm_ir_dumper
