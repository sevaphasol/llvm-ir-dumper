#pragma once

#include <string>

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm_ir_dump_pass {

class DumpPass : public llvm::PassInfoMixin<DumpPass> {
  private:
    std::string json_out_;
    std::string ll_out_;

  public:
    DumpPass( const std::string& json_out, const std::string& ll_out );

    llvm::PreservedAnalyses
    run( llvm::Module& module, llvm::ModuleAnalysisManager& /*unused*/ );

  private:
    void
    dumpModule( llvm::Module&         module,
                llvm::raw_fd_ostream& ll_dump,
                llvm::raw_fd_ostream& json_dump );
};

} // namespace llvm_ir_dump_pass
