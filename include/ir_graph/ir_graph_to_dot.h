#pragma once

#include <string>

#include "ir_graph/ir_graph.h"

namespace ir_graph {

std::string
renderDotGraph( const ir_graph::Graph& graph );

} // namespace ir_graph
