#pragma once

#include "llvm/Support/CommandLine.h"

static llvm::cl::opt<std::string>
    JsonOutBeforeOpt( "dumper-pass-json-out-before-opt",
                      llvm::cl::desc( "Output path for json dump before optimization" ) );

static llvm::cl::opt<std::string>
    JsonOutAfterOpt( "dumper-pass-json-out-after-opt",
                     llvm::cl::desc( "Output path for json dump after optimization" ) );

static llvm::cl::opt<std::string>
    IrOutBeforeOpt( "dumper-pass-ir-out-before-opt",
                    llvm::cl::desc( "Output path for ir dump before optimization" ) );

static llvm::cl::opt<std::string>
    IrOutAfterOpt( "dumper-pass-ir-out-after-opt",
                   llvm::cl::desc( "Output path for ir dump after optimization" ) );

static llvm::cl::opt<bool>
    EnableLoggingInjection( "dumper-pass-enable-logging-injection",
                            llvm::cl::desc( "Inject runtime logging instrumentation" ),
                            llvm::cl::init( false ) );
