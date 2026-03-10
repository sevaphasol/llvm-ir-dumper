#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm_ir_dumper {

class DumperPass : public llvm::PassInfoMixin<DumperPass> {
  private:
    const std::string_view dot_out_;
    const std::string_view ll_out_;

  private:
    void
    dumpModule( llvm::Module& M, llvm::raw_fd_ostream& ll_dump, llvm::raw_fd_ostream& dot_dump );

    void
    dumpFunc( llvm::Function&          F,
              llvm::ModuleSlotTracker& MST,
              llvm::raw_fd_ostream&    dot_dump,
              std::size_t&             func_cnt );

    void
    dumpBasicBlock( llvm::BasicBlock&        B,
                    llvm::ModuleSlotTracker& MST,
                    llvm::raw_fd_ostream&    dot_dump,
                    std::size_t              func_cnt,
                    std::size_t&             basic_block_cnt );

  public:
    DumperPass( const std::string& dot_out, const std::string& ll_out )
        : dot_out_( dot_out ), ll_out_( ll_out )
    {
    }

    llvm::PreservedAnalyses
    run( llvm::Module& M, llvm::ModuleAnalysisManager& AM );
};

} // namespace llvm_ir_dumper
