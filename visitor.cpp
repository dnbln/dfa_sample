//
// Created by Dinu on 12/2/2023.
//

#include "visitor.hpp"

void walk_expr(AstVisitor &visitor, const Expr &expr) {
    std::visit([&]<typename T0>(T0 &&arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, Name>) {
            visitor.visit_name(arg);
        } else if constexpr (std::is_same_v<T, Constant>) {
            visitor.visit_constant(arg);
        } else if constexpr (std::is_same_v<T, ParenExpr>) {
            visitor.visit_paren_expr(arg);
        } else if constexpr (std::is_same_v<T, BinaryExpr>) {
            visitor.visit_binary_expr(arg);
        } else {
            static_assert(false, "non-exhaustive visitor!");
        }
    }, expr.data);
}

void walk_paren_expr(AstVisitor &visitor, const ParenExpr &paren_expr) {
    visitor.visit_expr(*paren_expr.expr);
}

void walk_binary_expr(AstVisitor &visitor, const BinaryExpr &binary_expr) {
    visitor.visit_expr(*binary_expr.lhs);
    visitor.visit_expr(*binary_expr.rhs);
}

void walk_while_stmt(AstVisitor &visitor, const WhileStmt &while_stmt) {
    visitor.visit_expr(*while_stmt.condition);
    visitor.visit_stmt_list(*while_stmt.body);
}

void walk_if_stmt(AstVisitor &visitor, const IfStmt &if_stmt) {
    visitor.visit_expr(*if_stmt.condition);
    visitor.visit_stmt_list(*if_stmt.then_block);
}

void walk_assignment_stmt(AstVisitor &visitor, const AssignmentStmt &assignment_stmt) {
    visitor.visit_name(assignment_stmt.lhs);
    visitor.visit_expr(*assignment_stmt.rhs);
}

void walk_statement(AstVisitor &visitor, const Stmt &stmt) {
    std::visit([&]<typename T0>(T0 &&arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, AssignmentStmt>) {
            visitor.visit_assignment_stmt(arg);
        } else if constexpr (std::is_same_v<T, IfStmt>) {
            visitor.visit_if_stmt(arg);
        } else if constexpr (std::is_same_v<T, WhileStmt>) {
            visitor.visit_while_stmt(arg);
        } else {
            static_assert(false, "non-exhaustive visitor!");
        }
    }, stmt.data);
}

void walk_stmt_list(AstVisitor &visitor, const StmtList &stmt_list) {
    for (const Stmt &stmt: stmt_list.statements) {
        visitor.visit_statement(stmt);
    }
}

void walk_program(AstVisitor &visitor, const Program &program) {
    visitor.visit_stmt_list(program.statements);
}
