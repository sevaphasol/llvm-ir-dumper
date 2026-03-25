#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "ir_graph/ir_graph.h"

namespace ir_graph {

struct RuntimeProfile
{
    std::unordered_map<ir_graph::Id, std::uint64_t> function_execution_counts;
    std::unordered_map<ir_graph::Id, std::uint64_t> basic_block_execution_counts;
    std::unordered_map<ir_graph::Id, std::uint64_t> edge_execution_counts;

    bool
    empty() const;
};

RuntimeProfile
parseRuntimeProfile( std::string_view runtime_output );

void
applyRuntimeProfile( ir_graph::Graph& graph, const RuntimeProfile& profile );

} // namespace ir_graph
