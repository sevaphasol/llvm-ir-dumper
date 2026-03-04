#include "llvm/IR/Instruction.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<std::string> DotOut("dumper-pass-dot-out",
                                   cl::desc("Output path for dot dump"),
                                   cl::init("dumper_llvm_pass_out.dot"));

struct MyModPass : public PassInfoMixin<MyModPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
    std::error_code EC;
    raw_fd_ostream dump(DotOut, EC);
    if (EC) {
      errs() << "[dumper-pass] unable to open " << DotOut << ": "
             << EC.message() << "\n";
      return PreservedAnalyses::all();
    }

    ModuleSlotTracker MST(&M);

    outs() << "[Module] " << M.getName() << '\n';

    dump << "digraph \"" << M.getName() << "\" {\n"
         << "  graph [pad=0.3];\n"
         << "  rankdir=LR;\n"
         << "  newrank=true;\n"
         << "  splines=true;\n"
         << "  overlap=false;\n"
         << "  nodesep=0.35;\n"
         << "  ranksep=0.7;\n"
         << "  node  [fontname=\"SF Pro Text Bold 10\", fontsize=11];\n"
         << "  edge  [fontname=\"SF Pro Text Bold 10\", fontsize=10];\n";

    std::size_t func_cnt = 0;

    for (auto &F : M) {
      MST.incorporateFunction(F);

      outs() << "[Function] " << F.getName() << " (arg_size: " << F.arg_size()
             << ")\n";
      F.print(outs());
      outs() << "\n[Function Users]\n";
      for (auto &U : F.uses()) {
        User *user = U.getUser();
        user->print(outs(), true);
        outs() << '\n';
      }

      dump << "subgraph cluster_" << ++func_cnt << " {\n"
           << "  style=\"rounded\";\n"
           << "  color=\"#C0C0C0\";\n"
           << "  labelloc=\"t\";\n"
           << "  labeljust=\"l\";\n"
           << "  fontsize=13;\n"
           << "  fontname=\"SF Pro Text Bold 10\";\n"
           << "  label=<<B>" << F.getName() << "</B>>;\n";

      std::size_t basic_block_cnt = 0;

      for (auto &B : F) {
        outs() << "\n#[Basic block]";
        B.print(outs());
        outs() << "#[BasicBlock Users]\n";
        for (auto &U : B.uses()) {
          User *user = U.getUser();
          user->print(outs(), true);
          outs() << '\n';
        }

        for (auto &I : B) {
          outs() << "\n##[Instruction]\n";
          I.print(outs(), true);
          outs() << "\n##[Users]\n";
          for (auto &U : I.uses()) {
            User *user = U.getUser();
            user->print(outs(), true);
            outs() << '\n';
          }
          outs() << "##[Operands]\n";
          for (auto &U : I.operands()) {
            Value *use = U.get();
            use->print(outs(), true);
            outs() << '\n';
          }
        }

        basic_block_cnt++;

        dump << "subgraph cluster_" << func_cnt << "_" << basic_block_cnt
             << " {\n";
        dump << "  style=\"rounded\";\n";
        dump << "  color=\"#C0C0C0\";\n";
        dump << "  labelloc=\"t\";\n";
        dump << "  labeljust=\"l\";\n";
        dump << "  fontsize=13;\n";
        dump << "  fontname=\"SF Pro Text Bold 10\";\n";
        dump << "  label=<<B>" << "BasicBlock_" << basic_block_cnt
             << "</B>>;\n";

        for (auto &I : B) {
          dump << "n" << &I
               << "[shape=circle, style=\" filled \", fillcolor=\" #E8EEF9 \", "
                  "color=\" #1f1d31ff \", fontcolor=\" #000000 \", label=\""
               << I.getOpcodeName() << "\"];\n";
        }

        for (auto I_it = B.begin(); I_it != B.end(); ++I_it) {
          if (std::next(I_it, 1) == B.end()) {
            break;
          }
          dump << "n" << &(*I_it) << "->" << "n" << &(*std::next(I_it, 1))
               << " [color=\"#FF0000\"];\n";
        }

        for (auto I_it = B.begin(); I_it != B.end(); ++I_it) {
          for (auto &U : I_it->uses()) {
            User *user = U.getUser();
            int Slot = MST.getLocalSlot(&(*I_it));
            dump << "n" << &(*I_it) << "->" << "n" << user
                 << " [color=\"#0000FF\", label=\"%" << Slot << "\"];\n";
          }

          for (auto &U : I_it->operands()) {
            Value *use = U.get();

            if (!dyn_cast<Instruction, Value>(use)) {
              dump << "n" << use
                   << "[shape=circle, style=\" filled \", fillcolor =\" "
                      "#00EEF9 \", color=\" #1f1d31ff \", fontcolor=\" # "
                      "000000 \", label=\"";
              if (auto ConstInt = dyn_cast<ConstantInt>(use)) {
                use->getType()->print(dump);
                dump << " ";
                ConstInt->getValue().print(dump, true);
              } else {
                use->printAsOperand(dump);
              }
              dump << "\"];\n";
            }

            int Slot = MST.getLocalSlot(use);
            dump << "n" << use << "->" << "n" << &(*I_it)
                 << " [color=\"#00FF00\", label=\"%" << Slot << "\"];\n";
          }
        }

        dump << "}\n";
      }

      dump << "}\n";
    }

    dump << "}\n";

    return PreservedAnalyses::all();
  };
};

PassPluginLibraryInfo getPassPluginInfo() {
  const auto callback = [](PassBuilder &PB) {
    PB.registerPipelineStartEPCallback([](ModulePassManager &MPM, auto) {
      MPM.addPass(MyModPass{});
      return true;
    });
    // PB.registerOptimizerLastEPCallback([](ModulePassManager &MPM, auto) {
    //   MPM.addPass(MyModPass{});
    //   return true;
    // });
  };

  return {LLVM_PLUGIN_API_VERSION, "MyPlugin", "0.0.1", callback};
};

/* When a plugin is loaded by the driver, it will call this entry point to
obtain information about this plugin and about how to register its passes.
*/
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getPassPluginInfo();
}
