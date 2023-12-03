//
// Created by Dinu on 12/2/2023.
//

#ifndef DFA_SAMPLE_CFG_HPP
#define DFA_SAMPLE_CFG_HPP

#include <utility>
#include <variant>
#include <optional>
#include <ranges>
#include "ast.hpp"
#include "visitor.hpp"

struct CfgNode;

struct AssignmentCfgNode {
    Name name;
    std::shared_ptr<Expr> expr;
    Span span;
};

struct IfCfgNode {
    std::shared_ptr<Expr> condition;
    std::shared_ptr<CfgNode> then_branch;
};

struct WhileCfgNode {
    std::shared_ptr<Expr> condition;
    std::shared_ptr<CfgNode> body;
};

struct WhileRetDummyCfgNode {
    std::optional<std::weak_ptr<CfgNode>> while_node;
};

struct ExitCfgNode {
};

struct BasicCfgBlock {
    // nothing else in basic blocks except for assignments
    std::vector<std::shared_ptr<AssignmentCfgNode>> assignments;
};

struct CfgNode {
    std::variant<
        BasicCfgBlock,
        IfCfgNode,
        std::shared_ptr<WhileCfgNode>,
        std::shared_ptr<WhileRetDummyCfgNode>,
        ExitCfgNode> node;
    std::shared_ptr<CfgNode> next;
};

struct Cfg {
    std::shared_ptr<CfgNode> entry;
};

class CfgBuilder final : public AstVisitor {
    Cfg cfg;
    bool allow_direct_basic_link = true;

private:
    explicit CfgBuilder(Cfg cfg) : cfg(std::move(cfg)) {
    };

    friend Cfg build_cfg(const Program& program);

public:
    void visit_assignment_stmt(const AssignmentStmt& assignment_stmt) override {
        std::visit([&]<typename T0>(T0&& node) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, BasicCfgBlock>) {
                if (allow_direct_basic_link) {
                    node.assignments.insert(node.assignments.begin(),
                                            std::make_shared<AssignmentCfgNode>(AssignmentCfgNode{
                                                .name = assignment_stmt.lhs,
                                                .expr = assignment_stmt.rhs,
                                                .span = assignment_stmt.span,
                                            }));
                } else {
                    allow_direct_basic_link = true;
                    cfg.entry = std::make_shared<CfgNode>(CfgNode{
                        .node = BasicCfgBlock{
                            .assignments = {
                                std::make_shared<AssignmentCfgNode>(AssignmentCfgNode{
                                    .name = assignment_stmt.lhs,
                                    .expr = assignment_stmt.rhs,
                                    .span = assignment_stmt.span,
                                })
                            }
                        },
                        .next = cfg.entry
                    });
                }
            } else {
                cfg.entry = std::make_shared<CfgNode>(CfgNode{
                    .node = BasicCfgBlock{
                        .assignments = {
                            std::make_shared<AssignmentCfgNode>(AssignmentCfgNode{
                                .name = assignment_stmt.lhs,
                                .expr = assignment_stmt.rhs,
                                .span = assignment_stmt.span,
                            })
                        }
                    },
                    .next = cfg.entry
                });
            }
        }, cfg.entry->node);
    }

    void visit_if_stmt(const IfStmt& if_stmt) override {
        auto inner_cfg_builder = CfgBuilder{
            Cfg{
                .entry = cfg.entry,
            }
        };
        inner_cfg_builder.allow_direct_basic_link = false;
        inner_cfg_builder.visit_stmt_list(*if_stmt.then_block);
        cfg.entry = std::make_shared<CfgNode>(
            CfgNode{
                .node = IfCfgNode{
                    .condition = if_stmt.condition,
                    .then_branch = inner_cfg_builder.cfg.entry,
                },
                .next = cfg.entry
            }
        );
    }

    void visit_while_stmt(const WhileStmt& while_stmt) override {
        const std::shared_ptr<CfgNode> after_while = cfg.entry;
        std::shared_ptr<WhileRetDummyCfgNode> dummy_ret;
        cfg.entry = std::make_shared<CfgNode>(CfgNode{
            .node = dummy_ret = std::make_shared<WhileRetDummyCfgNode>(WhileRetDummyCfgNode{
                        .while_node = std::optional<std::weak_ptr<CfgNode>>(),
                    }),
            .next = cfg.entry
        });
        auto inner_cfg_builder = CfgBuilder{
            Cfg{
                .entry = cfg.entry,
            }
        };
        inner_cfg_builder.visit_stmt_list(*while_stmt.body);
        auto while_node = std::make_shared<WhileCfgNode>(WhileCfgNode{
            .condition = while_stmt.condition,
            .body = inner_cfg_builder.cfg.entry,
        });
        cfg.entry = std::make_shared<CfgNode>(
            CfgNode{
                .node = while_node,
                .next = after_while,
            }
        );
        dummy_ret->while_node = cfg.entry;
    }

    void visit_stmt_list(const StmtList& stmt_list) override {
        for (const auto& stmt: std::ranges::reverse_view(stmt_list.statements)) {
            this->visit_statement(stmt);
        }
    }
};

Cfg build_cfg(const Program& program);

void dbg_cfg(const Cfg& cfg);

void dbg_cfg_node(const CfgNode& node);

void dbg_cfg_node_at_indent(const CfgNode& node, int indent);

#endif //DFA_SAMPLE_CFG_HPP
