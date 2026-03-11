#include <string>
#include <string_view>
#include <unordered_map>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"

namespace dot_graph {
class Graph;
class Subgraph;
} // namespace dot_graph

namespace llvm_ir_dumper {

class DumperPass : public llvm::PassInfoMixin<DumperPass> {
  private:
    std::string dot_out_;
    std::string ll_out_;

    struct FuncInfoEntry
    {
        std::string                cluster_id;
        std::optional<std::string> anchor_id;
    };

    std::unordered_map<const llvm::Function*, FuncInfoEntry> funcs_info_;

  private:
    struct EdgeInfo
    {
        std::string_view color;
        std::size_t      weight;
        std::size_t      pen_width;
    };

    static constexpr EdgeInfo ControlFlowEdge = {
        .color     = "#ff8800",
        .weight    = 10000,
        .pen_width = 5,
    };

    static constexpr EdgeInfo InstrSeqEdge = {
        .color     = "#ff3636",
        .weight    = 1000,
        .pen_width = 3,
    };

    static constexpr EdgeInfo DataFlowEdge = {
        .color     = "#00b3ff",
        .weight    = 1,
        .pen_width = 2,
    };

    struct NodeInfo
    {
        std::string_view fill_color;
        std::string_view color;
        std::string_view font_color;
        std::string_view shape;
        std::string_view style;
    };

    static constexpr NodeInfo OperandNode = {
        .fill_color = "#d66868",
        .color      = "#000000",
        .font_color = "#000000",
        .shape      = "box",
        .style      = "rounded, filled",
    };

    static constexpr NodeInfo InstrNode = {
        .fill_color = "#6d95df",
        .color      = "#000000",
        .font_color = "#000000",
        .shape      = "box",
        .style      = "rounded, filled",
    };

    struct SubgraphInfo
    {
        std::string_view fill_color;
        std::string_view color;
        std::size_t      pen_width;
        std::size_t      font_size;
        std::string_view font_name;
        std::string_view shape;
        std::string_view style;
    };

    static constexpr SubgraphInfo FunctionSubgraph = {
        .fill_color = "#e3e3e3",
        .color      = "#000000",
        .pen_width  = 2,
        .font_size  = 20,
        .font_name  = "SF Pro Text Bold 10",
        .shape      = "box",
        .style      = "rounded, filled",
    };

    static constexpr SubgraphInfo BasicBlockSubgraph = {
        .fill_color = "#a4a4a4",
        .color      = "#000000",
        .pen_width  = 2,
        .font_size  = 15,
        .font_name  = "SF Pro Text Bold 10",
        .shape      = "box",
        .style      = "rounded, filled",
    };

    struct GraphInfo
    {
        std::string_view pad;
        std::string_view rankdir;
        bool             compound;
        bool             overlap;
        std::string_view nodesep;
        std::string_view ranksep;
        std::string_view node_font_name;
        std::size_t      node_font_size;
        std::string_view edge_font_name;
        std::size_t      edge_font_size;
        std::string_view splines;
    };

    static constexpr GraphInfo GraphInfo = {
        .pad            = "0.3",
        .rankdir        = "TB",
        .compound       = true,
        .overlap        = false,
        .nodesep        = "0.9",
        .ranksep        = "1.3",
        .node_font_name = "SF Pro Text Bold 10",
        .node_font_size = 15,
        .edge_font_name = "SF Pro Text Bold 10",
        .edge_font_size = 15,
        .splines        = "true",
    };

  public:
    DumperPass( const std::string& dot_out, const std::string& ll_out );

    llvm::PreservedAnalyses
    run( llvm::Module& module, llvm::ModuleAnalysisManager& /*unused*/ );

  private:
    static std::string
    formatOperandLabel( const llvm::Value& value );

    static std::string
    formatConstLabel( const llvm::Value& value );

    std::string
    formatSlotLabel( int slot );

    static std::string
    getNodeId( std::string_view prefix, const void* ptr );

    static std::string
    getValueNodeId( const llvm::Value* value );

    static std::string
    getUseNodeId( const llvm::Use* use );

    static std::string
    getFunctionClusterId( std::size_t func_index );

    static std::string
    getBasicBlockClusterId( std::size_t func_index, std::size_t basic_block_index );

    void
    dumpModule( llvm::Module&         module,
                llvm::raw_fd_ostream& ll_dump,
                llvm::raw_fd_ostream& dot_dump );

    void
    dumpFunc( llvm::Function&          func,
              llvm::ModuleSlotTracker& slot_tracker,
              dot_graph::Graph&        graph,
              std::size_t              func_index );

    void
    fillFuncsInfo( llvm::Module& module );

    void
    dumpFuncs( llvm::Module& module, dot_graph::Graph& graph );

    void
    dumpBasicBlocks( llvm::Function&          func,
                     llvm::ModuleSlotTracker& slot_tracker,
                     dot_graph::Graph&        graph,
                     dot_graph::Subgraph&     function_subgraph,
                     std::size_t              func_index );

    void
    dumpControlFlowEdges( llvm::Function& func, dot_graph::Graph& graph );

    void
    dumpControlFlowEdgesWithoutPhi( llvm::BasicBlock& basic_block, dot_graph::Graph& graph );

    void
    dumpControlFlowEdgesWithPhi( llvm::BasicBlock&                         basic_block,
                                 dot_graph::Graph&                         graph,
                                 llvm::BranchInst::InstListType::iterator& first_non_phi_it );

    void
    dumpBasicBlock( llvm::BasicBlock&        basic_block,
                    llvm::ModuleSlotTracker& slot_tracker,
                    dot_graph::Graph&        graph,
                    dot_graph::Subgraph&     block_subgraph );

    void
    dumpInstrNodes( llvm::BasicBlock& basic_block, dot_graph::Subgraph& block_subgraph );

    void
    dumpInstrSeqEdges( llvm::BasicBlock& basic_block, dot_graph::Graph& graph );

    void
    dumpDataFlowEdges( llvm::BasicBlock&        basic_block,
                       llvm::ModuleSlotTracker& slot_tracker,
                       dot_graph::Graph&        graph,
                       dot_graph::Subgraph&     block_subgraph );

    void
    dumpInstrUsers( llvm::BasicBlock&        basic_block,
                    llvm::Instruction*       instr,
                    llvm::ModuleSlotTracker& slot_tracker,
                    dot_graph::Graph&        graph,
                    bool&                    processed_call );

    void
    dumpInstrOperands( llvm::Instruction*       instr,
                       llvm::ModuleSlotTracker& slot_tracker,
                       dot_graph::Graph&        graph,
                       dot_graph::Subgraph&     block_subgraph,
                       bool                     processed_call );

    void
    dumpCallInstr( llvm::CallBase* call_base, dot_graph::Graph& graph, bool& processed_call );
};

} // namespace llvm_ir_dumper
