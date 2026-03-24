#pragma once

#include <string>

#include "ir_graph/ir_graph.h"

namespace llvm_ir_dumper {

std::string
renderDotGraph( const ir_graph::Graph& graph );

} // namespace llvm_ir_dumper
