//
// Created by Dinu on 12/2/2023.
//

#ifndef DFA_SAMPLE_VISITOR_HPP
#define DFA_SAMPLE_VISITOR_HPP

#include "ast.hpp"

struct AstVisitor;

void walk_expr(AstVisitor &visitor, const Expr &expr);

void walk_paren_expr(AstVisitor &visitor, const ParenExpr &paren_expr);

void walk_binary_expr(AstVisitor &visitor, const BinaryExpr &binary_expr);

void walk_while_stmt(AstVisitor &visitor, const WhileStmt &while_stmt);

void walk_if_stmt(AstVisitor &visitor, const IfStmt &if_stmt);

void walk_assignment_stmt(AstVisitor &visitor, const AssignmentStmt &assignment_stmt);

void walk_statement(AstVisitor &visitor, const Stmt &stmt);

void walk_stmt_list(AstVisitor &visitor, const StmtList &stmt_list);

void walk_program(AstVisitor &visitor, const Program &program);

struct AstVisitor {
    virtual ~AstVisitor() = default;

    virtual void visit_name(const Name &name) {
    }

    virtual void visit_constant(const Constant &constant) {
    }

    virtual void visit_expr(const Expr &expr) {
        walk_expr(*this, expr);
    }

    virtual void visit_paren_expr(const ParenExpr &paren_expr) {
        walk_paren_expr(*this, paren_expr);
    }

    virtual void visit_binary_expr(const BinaryExpr &binary_expr) {
        walk_binary_expr(*this, binary_expr);
    }

    virtual void visit_statement(const Stmt &stmt) {
        walk_statement(*this, stmt);
    }

    virtual void visit_assignment_stmt(const AssignmentStmt &assignment_stmt) {
        walk_assignment_stmt(*this, assignment_stmt);
    }

    virtual void visit_if_stmt(const IfStmt &if_stmt) {
        walk_if_stmt(*this, if_stmt);
    }

    virtual void visit_while_stmt(const WhileStmt &while_stmt) {
        walk_while_stmt(*this, while_stmt);
    }

    virtual void visit_stmt_list(const StmtList &stmt_list) {
        walk_stmt_list(*this, stmt_list);
    }

    virtual void visit_program(const Program &program) {
        walk_program(*this, program);
    }
};

#endif //DFA_SAMPLE_VISITOR_HPP
