#include "llvm/IR/Constants.h"
#include "llvm/IR/ModuleSlotTracker.h"

#include "llvm_ir_dumper/dumper_pass.h"

namespace llvm_ir_dumper {

llvm::PreservedAnalyses
DumperPass::run( llvm::Module& M, llvm::ModuleAnalysisManager& AM )
{
    std::error_code      EC;
    llvm::raw_fd_ostream dump( dot_out, EC );
    if ( EC )
    {
        llvm::errs() << "[dumper-pass] unable to open " << dot_out << ": " << EC.message()
                     << "\n";
        return llvm::PreservedAnalyses::all();
    }

    llvm::ModuleSlotTracker MST( &M );

    llvm::outs() << "[Module] " << M.getName() << '\n';

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

    for ( auto& F : M )
    {
        MST.incorporateFunction( F );

        llvm::outs() << "[Function] " << F.getName() << " (arg_size: " << F.arg_size() << ")\n";
        F.print( llvm::outs() );
        llvm::outs() << "\n[Function Users]\n";
        for ( auto& U : F.uses() )
        {
            llvm::User* user = U.getUser();
            user->print( llvm::outs(), true );
            llvm::outs() << '\n';
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

        for ( auto& B : F )
        {
            llvm::outs() << "\n#[Basic block]";
            B.print( llvm::outs() );
            llvm::outs() << "#[BasicBlock Users]\n";
            for ( auto& U : B.uses() )
            {
                llvm::User* user = U.getUser();
                user->print( llvm::outs(), true );
                llvm::outs() << '\n';
            }

            for ( auto& I : B )
            {
                llvm::outs() << "\n##[Instruction]\n";
                I.print( llvm::outs(), true );
                llvm::outs() << "\n##[Users]\n";
                for ( auto& U : I.uses() )
                {
                    llvm::User* user = U.getUser();
                    user->print( llvm::outs(), true );
                    llvm::outs() << '\n';
                }
                llvm::outs() << "##[Operands]\n";
                for ( auto& U : I.operands() )
                {
                    llvm::Value* use = U.get();
                    use->print( llvm::outs(), true );
                    llvm::outs() << '\n';
                }
            }

            basic_block_cnt++;

            dump << "subgraph cluster_" << func_cnt << "_" << basic_block_cnt << " {\n";
            dump << "  style=\"rounded\";\n";
            dump << "  color=\"#C0C0C0\";\n";
            dump << "  labelloc=\"t\";\n";
            dump << "  labeljust=\"l\";\n";
            dump << "  fontsize=13;\n";
            dump << "  fontname=\"SF Pro Text Bold 10\";\n";
            dump << "  label=<<B>" << "BasicBlock_" << basic_block_cnt << "</B>>;\n";

            for ( auto& I : B )
            {
                dump << "n" << &I
                     << "[shape=circle, style=\" filled \", fillcolor=\" #E8EEF9 \", "
                        "color=\" #1f1d31ff \", fontcolor=\" #000000 \", label=\""
                     << I.getOpcodeName() << "\"];\n";
            }

            for ( auto I_it = B.begin(); I_it != B.end(); ++I_it )
            {
                if ( std::next( I_it, 1 ) == B.end() )
                {
                    break;
                }
                dump << "n" << &( *I_it ) << "->" << "n" << &( *std::next( I_it, 1 ) )
                     << " [color=\"#FF0000\"];\n";
            }

            for ( auto I_it = B.begin(); I_it != B.end(); ++I_it )
            {
                for ( auto& U : I_it->uses() )
                {
                    llvm::User* user = U.getUser();
                    int         Slot = MST.getLocalSlot( &( *I_it ) );
                    dump << "n" << &( *I_it ) << "->" << "n" << user
                         << " [color=\"#0000FF\", label=\"%" << Slot << "\"];\n";
                }

                for ( auto& U : I_it->operands() )
                {
                    llvm::Value* use = U.get();

                    if ( !llvm::dyn_cast<llvm::Instruction, llvm::Value>( use ) )
                    {
                        dump << "n" << use
                             << "[shape=circle, style=\" filled \", fillcolor =\" "
                                "#00EEF9 \", color=\" #1f1d31ff \", fontcolor=\" # "
                                "000000 \", label=\"";
                        if ( auto ConstInt = llvm::dyn_cast<llvm::ConstantInt>( use ) )
                        {
                            use->getType()->print( dump );
                            dump << " ";
                            ConstInt->getValue().print( dump, true );
                        } else
                        {
                            use->printAsOperand( dump );
                        }
                        dump << "\"];\n";
                    }

                    int Slot = MST.getLocalSlot( use );
                    dump << "n" << use << "->" << "n" << &( *I_it )
                         << " [color=\"#00FF00\", label=\"%" << Slot << "\"];\n";
                }
            }

            dump << "}\n";
        }

        dump << "}\n";
    }

    dump << "}\n";

    return llvm::PreservedAnalyses::all();
};

} // namespace llvm_ir_dumper
