//
// Created by Dinu on 12/2/2023.
//

#include "cfg.hpp"

Cfg build_cfg(const Program& program) {
    auto cfg_builder = CfgBuilder{
        Cfg{
            .entry = std::make_shared<CfgNode>(CfgNode{
                .node = ExitCfgNode(),
                .next = nullptr
            })
        }
    };
    cfg_builder.visit_program(program);
    return cfg_builder.cfg;
}

void print_indent(const int indent) {
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }
}

static void dbg_expr(const Expr& expr) {
    std::visit(
        [&]<typename T0>(T0&& expr_data) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, Name>) {
                std::cout << expr_data.name;
            } else if constexpr (std::is_same_v<T, Constant>) {
                std::cout << expr_data.value;
            } else if constexpr (std::is_same_v<T, ParenExpr>) {
                std::cout << "(";
                dbg_expr(*expr_data.expr);
                std::cout << ")";
            } else if constexpr (std::is_same_v<T, BinaryExpr>) {
                std::cout << "{(";
                dbg_expr(*expr_data.lhs);
                const BinaryOp op = expr_data.op;
                std::cout << " ";
                switch (op) {
                    case BinaryOp::Add:
                        std::cout << "+";
                        break;
                    case BinaryOp::Sub:
                        std::cout << "-";
                        break;
                    case BinaryOp::Mul:
                        std::cout << "*";
                        break;
                    case BinaryOp::Div:
                        std::cout << "/";
                        break;
                    case BinaryOp::Lt:
                        std::cout << "<";
                        break;
                    case BinaryOp::Gt:
                        std::cout << ">";
                        break;
                }
                std::cout << " ";
                dbg_expr(*expr_data.rhs);
                std::cout << ")}";
            } else {
                static_assert(false, "non-exhaustive visitor!");
            }
        }, expr.data);
}

static void dbg_cfg_basic_block(const BasicCfgBlock& block, const int indent) {
    print_indent(indent);
    std::cout << "basic_block" << std::endl;
    for (const auto& assignment: block.assignments) {
        print_indent(indent + 1);
        std::cout << assignment->name.name << " = ";
        dbg_expr(*assignment->expr);
        std::cout << std::endl;
    }
}

static void dbg_cfg_node_impl(const CfgNode& node, const int indent, const CfgNode* stop_at) {
    if (&node == stop_at)
        return;
    std::visit(
        [&]<typename T0>(T0&& cfg_node) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, BasicCfgBlock>) {
                dbg_cfg_basic_block(cfg_node, indent);
                dbg_cfg_node_impl(*node.next, indent, stop_at);
            } else if constexpr (std::is_same_v<T, IfCfgNode>) {
                print_indent(indent);
                std::cout << "if ";
                dbg_expr(*cfg_node.condition);
                std::cout << std::endl;
                dbg_cfg_node_impl(*cfg_node.then_branch, indent + 1, node.next.get());
                print_indent(indent);
                std::cout << "end" << std::endl;
                dbg_cfg_node_impl(*node.next, indent, stop_at);
            } else if constexpr (std::is_same_v<T, std::shared_ptr<WhileCfgNode>>) {
                print_indent(indent);
                std::cout << "while ";
                dbg_expr(*cfg_node->condition);
                std::cout << std::endl;
                dbg_cfg_node_impl(*cfg_node->body, indent + 1, stop_at);
                print_indent(indent);
                std::cout << "end" << std::endl;
            } else if constexpr (std::is_same_v<T, std::shared_ptr<WhileRetDummyCfgNode>>) {
                print_indent(indent);
                std::cout << "while_ret_dummy" << std::endl;
            } else if constexpr (std::is_same_v<T, ExitCfgNode>) {
                print_indent(indent);
                std::cout << "exit" << std::endl;
            } else {
                static_assert(false, "non-exhaustive visitor!");
            }
        }, node.node);
}

void dbg_cfg(const Cfg& cfg) {
    dbg_cfg_node_impl(*cfg.entry, 0, nullptr);
}

void dbg_cfg_node(const CfgNode& node) {
    dbg_cfg_node_at_indent(node, 0);
}

void dbg_cfg_node_at_indent(const CfgNode& node, const int indent) {
    dbg_cfg_node_impl(node, indent, node.next.get());
}
