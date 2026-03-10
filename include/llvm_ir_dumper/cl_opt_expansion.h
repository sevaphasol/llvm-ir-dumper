#pragma once

#include "llvm/Support/CommandLine.h"

static llvm::cl::opt<std::string>
    DotOutBeforeOpt( "dumper-pass-dot-out-before-opt",
                     llvm::cl::desc( "Output path for dot dump before optimization" ) );

static llvm::cl::opt<std::string>
    DotOutAfterOpt( "dumper-pass-dot-out-after-opt",
                    llvm::cl::desc( "Output path for dot dump after optimization" ) );
