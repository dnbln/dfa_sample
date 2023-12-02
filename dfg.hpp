//
// Created by Dinu on 12/2/2023.
//

#ifndef DFA_SAMPLE_DFG_HPP
#define DFA_SAMPLE_DFG_HPP

#include <vector>
#include <set>
#include "cfg.hpp"

class DfgNode {
public:
    std::vector<std::weak_ptr<DfgNode>> in_nodes;
    std::vector<std::weak_ptr<DfgNode>> out_nodes;
    std::shared_ptr<CfgNode> cfg_node;

    DfgNode() = default;
    DfgNode(const DfgNode &) = default;
    DfgNode(DfgNode &&) = default;
    DfgNode(std::shared_ptr<CfgNode> cfg_node) : cfg_node(std::move(cfg_node)) {}
    DfgNode(std::shared_ptr<CfgNode> cfg_node,
            std::vector<std::weak_ptr<DfgNode>> in_nodes,
            std::vector<std::weak_ptr<DfgNode>> out_nodes) :
            cfg_node(std::move(cfg_node)), in_nodes(std::move(in_nodes)),
            out_nodes(std::move(out_nodes)) {}
};

class Dfg {
    std::shared_ptr<DfgNode> exit_node;
public:
    std::vector<std::shared_ptr<DfgNode>> nodes;
};

std::shared_ptr<DfgNode> dfg_ptr_for_cfg(const Dfg &dfg, const CfgNode &cfg_node);

void dbg_dfg(const Dfg &dfg);
Dfg build_dfg(const Cfg &cfg);

#endif //DFA_SAMPLE_DFG_HPP
