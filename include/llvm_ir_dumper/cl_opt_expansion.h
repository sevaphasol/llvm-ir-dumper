#pragma once

#include "llvm/Support/CommandLine.h"

static llvm::cl::opt<std::string>
    DotOutBeforeOpt( "dumper-pass-dot-out-before-opt",
                     llvm::cl::desc( "Output path for dot dump before optimization" ),
                     llvm::cl::init( "llvm_ir_dump_before_opt.dot" ) );

static llvm::cl::opt<std::string>
    DotOutAfterOpt( "dumper-pass-dot-out-after-opt",
                    llvm::cl::desc( "Output path for dot dump after optimization" ),
                    llvm::cl::init( "llvm_ir_dump_after_opt.dot" ) );
