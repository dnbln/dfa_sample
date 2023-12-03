//
// Created by Dinu on 12/2/2023.
//

#include "dfg.hpp"

std::shared_ptr<DfgNode> dfg_ptr_for_cfg(const Dfg& dfg, const CfgNode& cfg_node) {
    for (const auto& node: dfg.nodes) {
        if (node->cfg_node.get() == &cfg_node) {
            return node;
        }
    }
    throw std::runtime_error("Could not find dfg node for cfg node");
}

static void build_dfg_nodes(const std::shared_ptr<CfgNode>& cfg_node, Dfg& dfg) {
    dfg.nodes.push_back(std::make_shared<DfgNode>(DfgNode(cfg_node)));
    std::visit(
        [&]<typename T0>(T0&& node) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, BasicCfgBlock>
                          || std::is_same_v<T, std::shared_ptr<WhileRetDummyCfgNode>>) {
                build_dfg_nodes(cfg_node->next, dfg);
            } else if constexpr (std::is_same_v<T, IfCfgNode>) {
                build_dfg_nodes(node.then_branch, dfg);
                // will eventually reach outside the if, not for us to worry about
            } else if constexpr (std::is_same_v<T, std::shared_ptr<WhileCfgNode>>) {
                build_dfg_nodes(node->body, dfg);
            } else if constexpr (std::is_same_v<T, ExitCfgNode>) {
            } else {
                static_assert(false, "non-exhaustive visitor!");
            }
        }, cfg_node->node);
}

static void forward_link_dfg_nodes(const std::shared_ptr<CfgNode>& cfg_node, Dfg& dfg) {
    std::visit(
        [&]<typename T0>(T0&& node) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, BasicCfgBlock>) {
                const std::shared_ptr<DfgNode> dfg_node = dfg_ptr_for_cfg(dfg, *cfg_node);
                dfg_node->out_nodes.push_back(dfg_ptr_for_cfg(dfg, *cfg_node->next));
                forward_link_dfg_nodes(cfg_node->next, dfg);
            } else if constexpr (std::is_same_v<T, IfCfgNode>) {
                const std::shared_ptr<DfgNode> dfg_node = dfg_ptr_for_cfg(dfg, *cfg_node);
                dfg_node->out_nodes.push_back(dfg_ptr_for_cfg(dfg, *node.then_branch));
                dfg_node->out_nodes.push_back(dfg_ptr_for_cfg(dfg, *cfg_node->next));
                forward_link_dfg_nodes(node.then_branch, dfg);
            } else if constexpr (std::is_same_v<T, std::shared_ptr<WhileCfgNode>>) {
                const std::shared_ptr<DfgNode> dfg_node = dfg_ptr_for_cfg(dfg, *cfg_node);
                dfg_node->out_nodes.push_back(dfg_ptr_for_cfg(dfg, *node->body));
                dfg_node->out_nodes.push_back(dfg_ptr_for_cfg(dfg, *cfg_node->next));
                forward_link_dfg_nodes(node->body, dfg);
            } else if constexpr (std::is_same_v<T, std::shared_ptr<WhileRetDummyCfgNode>>) {
                const std::shared_ptr<DfgNode> dfg_node = dfg_ptr_for_cfg(dfg, *cfg_node);
                const std::optional<std::weak_ptr<CfgNode>> while_node = node->while_node;
                const std::shared_ptr<CfgNode> while_node_locked = while_node.value().lock();
                dfg_node->out_nodes.push_back(dfg_ptr_for_cfg(dfg, *while_node_locked));
                forward_link_dfg_nodes(cfg_node->next, dfg);
            } else if constexpr (std::is_same_v<T, ExitCfgNode>) {
            } else {
                static_assert(false, "non-exhaustive visitor!");
            }
        }, cfg_node->node);
}

static void backward_link_dfg_nodes(Dfg& dfg, std::shared_ptr<DfgNode> node,
                                    const std::optional<std::weak_ptr<DfgNode>>& prev) {
    if (prev.has_value()) {
        node->in_nodes.push_back(prev.value());
    }

    const auto prv = std::make_optional(node);

    for (const auto& out_node: node->out_nodes) {
        const std::shared_ptr<DfgNode> out_node_locked = out_node.lock();
        bool skip = false;
        for (const auto& in_node: out_node_locked->in_nodes) {
            if (in_node.lock().get() == node.get()) {
                skip = true;
                break;
            }
        }
        if (skip) continue;
        backward_link_dfg_nodes(dfg, out_node_locked, prv);
    }
}

Dfg build_dfg(const Cfg& cfg) {
    Dfg dfg;

    build_dfg_nodes(cfg.entry, dfg);
    forward_link_dfg_nodes(cfg.entry, dfg);
    backward_link_dfg_nodes(dfg, dfg_ptr_for_cfg(dfg, *cfg.entry), std::nullopt);

    return dfg;
}

void dbg_dfg(const Dfg& dfg) {
    for (size_t i = 0; i < dfg.nodes.size(); i++) {
        std::cout << "node " << i << ":" << std::endl;
        std::cout << "    cfg_node:\n";
        dbg_cfg_node_at_indent(*dfg.nodes[i]->cfg_node, 4);
        std::cout << "    in_nodes:\n";
        for (const auto& in_node: dfg.nodes[i]->in_nodes) {
            dbg_cfg_node_at_indent(*in_node.lock()->cfg_node, 8);
        }

        std::cout << "    out_nodes:\n";
        for (const auto& out_node: dfg.nodes[i]->out_nodes) {
            dbg_cfg_node_at_indent(*out_node.lock()->cfg_node, 8);
        }
    }
}
