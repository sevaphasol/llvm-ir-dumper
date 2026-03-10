#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Support/Casting.h"

#include "llvm_ir_dumper/dumper_pass.h"

namespace llvm_ir_dumper {

llvm::PreservedAnalyses
DumperPass::run( llvm::Module& M, llvm::ModuleAnalysisManager& AM )
{
    std::error_code EC;

    llvm::raw_fd_ostream dot_dump( dot_out_, EC );
    if ( EC )
    {
        llvm::errs() << "[dumper-pass] unable to open dot_dump file (" << dot_out_
                     << "): " << EC.message() << "\n";
        return llvm::PreservedAnalyses::all();
    }

    llvm::raw_fd_ostream ll_dump( ll_out_, EC );
    if ( EC )
    {
        llvm::errs() << "[dumper-pass] unable to open ll_dump file (" << ll_out_
                     << "): " << EC.message() << "\n";
        return llvm::PreservedAnalyses::all();
    }

    dumpModule( M, ll_dump, dot_dump );

    return llvm::PreservedAnalyses::all();
};

void
DumperPass::dumpModule( llvm::Module&         M,
                        llvm::raw_fd_ostream& ll_dump,
                        llvm::raw_fd_ostream& dot_dump )
{
    M.print( ll_dump, nullptr );

    llvm::ModuleSlotTracker MST( &M );

    dot_dump << "digraph \"" << M.getName() << "\" {\n"
             << "  graph [pad=0.3];\n"
             << "  rankdir=UB;\n"
             << "  newrank=true;\n"
             //  << "  splines=ortho;\n"
             << "  compound=true;\n"
             << "  overlap=false;\n"
             << "  nodesep=0.35;\n"
             << "  ranksep=0.7;\n"
             << "  node  [fontname=\"SF Pro Text Bold 10\", fontsize=11];\n"
             << "  edge  [fontname=\"SF Pro Text Bold 10\", fontsize=10];\n";

    std::size_t func_cnt = 0;

    for ( auto& F : M )
    {
        MST.incorporateFunction( F );
        dumpFunc( F, MST, dot_dump, func_cnt );
    }

    dot_dump << "}\n";
}

void
DumperPass::dumpFunc( llvm::Function&          F,
                      llvm::ModuleSlotTracker& MST,
                      llvm::raw_fd_ostream&    dot_dump,
                      std::size_t&             func_cnt )
{
    func_cnt++;

    dot_dump << "subgraph cluster_" << std::hash<std::string>{}( std::string( F.getName() ) )
             << " {\n"
             << "  shape=\"box\";\n"
             << "  style=\"rounded, filled\";\n"
             << "  color=\"#000000ff\";\n"
             << "  penwidth=2;\n"
             << "  fillcolor=\"#e3e3e3ff\";"
             << "  labelloc=\"t\";\n"
             << "  labeljust=\"l\";\n"
             << "  fontsize=13;\n"
             << "  fontname=\"SF Pro Text Bold 10\";\n"
             << "  label=<<B>" << F.getName() << "</B>>;\n";

    std::size_t basic_block_cnt = 0;

    for ( auto& B : F )
    {
        dumpBasicBlock( B, MST, dot_dump, func_cnt, basic_block_cnt );
    }

    for ( auto& B : F )
    {
        auto first_non_phi_it = B.getFirstNonPHIIt();
        if ( first_non_phi_it == B.begin() )
        {
            auto* first_inst = &*B.begin();
            for ( auto* pred : llvm::predecessors( &B ) )
            {
                dot_dump << "n" << pred->getTerminator() << "->" << "n" << first_inst
                         << " [color=\"#ea00ffff\", weight=10000, penwidth=5];\n";
            }
        } else
        {
            for ( auto I_it = B.begin(); I_it != first_non_phi_it; ++I_it )
            {
                auto* phi = llvm::dyn_cast<llvm::PHINode>( I_it );
                assert( phi != nullptr );

                std::size_t n_incoming_values = phi->getNumIncomingValues();
                for ( std::size_t i = 0; i < n_incoming_values; ++i )
                {
                    dot_dump << "n" << phi->getIncomingBlock( i )->getTerminator() << "->" << "n"
                             << phi
                             << " [color=\"#ea00ffff\", weight=10000, penwidth=5, label=\"";
                    phi->getIncomingValue( i )->printAsOperand( dot_dump );
                    dot_dump << "\"];\n";
                }
            }
        }
    }

    dot_dump << "}\n";
}

void
DumperPass::dumpBasicBlock( llvm::BasicBlock&        B,
                            llvm::ModuleSlotTracker& MST,
                            llvm::raw_fd_ostream&    dot_dump,
                            std::size_t              func_cnt,
                            std::size_t&             basic_block_cnt )
{
    const auto printOperandNodeId = []( llvm::raw_ostream& OS,
                                        const llvm::Value* V,
                                        const llvm::User*  user,
                                        unsigned           operand_index ) {
        if ( llvm::isa<llvm::Constant>( V ) && !llvm::isa<llvm::GlobalValue>( V ) )
        {
            OS << "nconst_" << user << "_" << operand_index;
        } else
        {
            OS << "n" << V;
        }
    };

    basic_block_cnt++;

    dot_dump << "subgraph cluster_" << func_cnt << "_" << basic_block_cnt << " {\n"
             << "  shape=\"box\";\n"
             << "  style=\"rounded, filled\";\n"
             << "  color=\"#000000ff\";\n"
             << "  penwidth=2;\n"
             << "  fillcolor=\"#a4a4a4ff\";"
             << "  labelloc=\"t\";\n"
             << "  labeljust=\"l\";\n"
             << "  fontsize=13;\n"
             << "  fontname=\"SF Pro Text Bold 10\";\n"
             << "  label=\"\";\n";

    for ( auto& I : B )
    {
        dot_dump << "n" << &I
                 << "[shape=box, style=\"rounded, filled\", fillcolor=\"#E8EEF9\", "
                    "color=\"#1f1d31ff\", fontcolor=\"#000000\", label=\""
                 << I.getOpcodeName() << "\"];\n";
    }

    for ( auto I_it = B.begin(); I_it != B.end(); ++I_it )
    {
        if ( std::next( I_it, 1 ) == B.end() )
        {
            break;
        }
        dot_dump << "n" << &( *I_it ) << "->" << "n" << &( *std::next( I_it, 1 ) )
                 << " [color=\"#FF0000\", weight=1000, penwidth=3];\n";
    }

    for ( auto I_it = B.begin(); I_it != B.end(); ++I_it )
    {
        for ( auto& U : I_it->uses() )
        {
            llvm::User* user = U.getUser();

            int slot = MST.getLocalSlot( &( *I_it ) );
            dot_dump << "n" << &( *I_it ) << "->" << "n" << user << " [color=\"#0000FF\"";
            if ( slot != -1 )
            {
                dot_dump << ", label=\"%" << slot << "\"";
            }

            dot_dump << "];\n";
        }

        bool no_dump_func_operand_for_call = false;

        if ( auto* CB = llvm::dyn_cast<llvm::CallBase>( I_it ) )
        {
            if ( auto* callee = CB->getCalledFunction() )
            {
                if ( !callee->isDeclaration() )
                {
                    no_dump_func_operand_for_call = true;

                    dot_dump << "n" << &*I_it << "->"
                             << "n" << &callee->getEntryBlock().front()
                             << " [color=\"#ff8800\", style=\"dashed\", penwidth=3"
                             << ", lhead=\"cluster_"
                             << std::hash<std::string>{}( std::string( callee->getName() ) )
                             << "\""
                             << ", constraint=false"
                             << "];\n";
                }
            }
        }

        unsigned operand_index = 0;

        for ( auto& U : I_it->operands() )
        {
            llvm::Value* use = U.get();

            if ( no_dump_func_operand_for_call && llvm::dyn_cast<llvm::Function>( use ) )
            {
                continue;
            }

            if ( llvm::dyn_cast<llvm::PHINode>( I_it ) )
            {
                continue;
            }

            if ( !llvm::dyn_cast<llvm::Instruction, llvm::Value>( use ) )
            {
                printOperandNodeId( dot_dump, use, &( *I_it ), operand_index );
                dot_dump << "[shape=box, style=\"rounded, filled\", fillcolor=\"#00EEF9\", "
                            "color=\"#1f1d31ff\", fontcolor=\"#000000\", label=\"";
                if ( auto ConstInt = llvm::dyn_cast<llvm::ConstantInt>( use ) )
                {
                    use->getType()->print( dot_dump );
                    dot_dump << " ";
                    ConstInt->getValue().print( dot_dump, true );
                } else
                {
                    use->printAsOperand( dot_dump );
                }
                dot_dump << "\"];\n";
            }

            int slot = MST.getLocalSlot( use );
            printOperandNodeId( dot_dump, use, &( *I_it ), operand_index );
            dot_dump << "->"
                     << "n" << &( *I_it ) << " [color=\"#00FF00\"";
            if ( slot != -1 )
            {
                dot_dump << ", label=\"%" << slot << "\"";
            }

            dot_dump << "];\n";
            operand_index++;
        }
    }

    dot_dump << "}\n";
}

} // namespace llvm_ir_dumper
