//
// Created by Dinu on 12/2/2023.
//

#ifndef DFA_SAMPLE_AST_HPP
#define DFA_SAMPLE_AST_HPP

#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <memory>

struct Span {
    size_t start;
    size_t end;

    bool operator==(const Span &other) const {
        return start == other.start && end == other.end;
    }
};

struct Name {
    std::string_view name;
    Span span;

    bool operator==(const Name &other) const {
        return name == other.name;
    }

    bool operator==(const char *s) const {
        return name == s;
    }

    bool operator<(const Name &other) const {
        return name < other.name;
    }
};

struct Constant {
    int value;
    Span span;

    bool operator==(const Constant &other) const {
        return value == other.value;
    }
};

struct Expr;

struct ParenExpr {
    std::shared_ptr<Expr> expr;
    Span span;
};

enum BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Lt,
    Gt
};

struct BinaryExpr {
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;
    BinaryOp op;
    Span span;
};

struct Expr {
    std::variant<
            Name,
            Constant,
            ParenExpr,
            BinaryExpr
    > data;
};

struct StmtList;

struct AssignmentStmt {
    Name lhs;
    std::shared_ptr<Expr> rhs;
    Span span;
};

struct IfStmt {
    std::shared_ptr<Expr> condition;
    std::shared_ptr<StmtList> then_block;
};

struct WhileStmt {
    std::shared_ptr<Expr> condition;
    std::shared_ptr<StmtList> body;
};

struct Stmt {
    std::variant<
            AssignmentStmt,
            IfStmt,
            WhileStmt
    > data;
};

struct StmtList {
    std::vector<Stmt> statements;
};

struct Program {
    StmtList statements;
};


#endif //DFA_SAMPLE_AST_HPP
