#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm_ir_inject_pass {

class InjectPass : public llvm::PassInfoMixin<InjectPass> {
  public:
    llvm::PreservedAnalyses
    run( llvm::Module& module, llvm::ModuleAnalysisManager& /*unused*/ );

  private:
    void
    injectLogging( llvm::Module& module );
};

} // namespace llvm_ir_inject_pass
