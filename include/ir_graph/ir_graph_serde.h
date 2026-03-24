#pragma once

#include <iosfwd>
#include <string>

#include "ir_graph/ir_graph.h"

namespace ir_graph {

void
to_json( nlohmann::json& json, const Function& function );

void
from_json( const nlohmann::json& json, Function& function );

void
to_json( nlohmann::json& json, const BasicBlock& basic_block );

void
from_json( const nlohmann::json& json, BasicBlock& basic_block );

void
to_json( nlohmann::json& json, const Node& node );

void
from_json( const nlohmann::json& json, Node& node );

void
to_json( nlohmann::json& json, const Edge& edge );

void
from_json( const nlohmann::json& json, Edge& edge );

void
to_json( nlohmann::json& json, const Graph& graph );

void
from_json( const nlohmann::json& json, Graph& graph );

std::string
serialize( const Graph& graph, int indent = 2 );

Graph
deserialize( std::istream& input );

} // namespace ir_graph
