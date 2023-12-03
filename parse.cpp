//
// Created by Dinu on 12/3/2023.
//

#include "parse.hpp"

std::shared_ptr<Expr> parse_expr(ParserState &state);

std::shared_ptr<Expr> parse_precedence_1(ParserState &state);

std::shared_ptr<Expr> parse_precedence_2(ParserState &state);

std::shared_ptr<Expr> parse_precedence_3(ParserState &state);

Expr parse_expr_atom(ParserState &state) {
    state.lexer.skip_whitespace();

    const char c = state.lexer.peek();
    if (c == '(') {
        auto start = state.lexer.pos;
        state.lexer.next();
        std::shared_ptr<Expr> expr = parse_expr(state);
        state.lexer.skip_whitespace();

        if (state.lexer.eof()) {
            throw std::runtime_error("Unexpected end of input");
        }

        if (state.lexer.next() != ')') {
            throw std::runtime_error("Expected ')'");
        }
        return Expr{
                .data = ParenExpr{
                        .expr = expr,
                        .span = Span{start, state.lexer.pos}
                }
        };
    }
    if (isdigit(c)) {
        return Expr{
            state.lexer.read_number()
        };
    }
    if (isalpha(c)) {
        return Expr{
            state.lexer.read_name()
        };
    }
    throw std::runtime_error("Unexpected character");
}

std::shared_ptr<Expr> parse_precedence_1(ParserState &state) {
    std::shared_ptr<Expr> lhs = std::make_shared<Expr>(parse_expr_atom(state));
    state.lexer.skip_whitespace();

    if (state.lexer.eof()) {
        return std::move(lhs);
    }

    char c = state.lexer.peek();
    while (c == '*' || c == '/') {
        const auto start = state.lexer.pos;
        state.lexer.next();
        std::shared_ptr<Expr> rhs = std::make_shared<Expr>(parse_expr_atom(state));
        lhs = std::make_shared<Expr>(Expr{
                .data = BinaryExpr{
                        .lhs = lhs,
                        .rhs = rhs,
                        .op = c == '*' ? Mul : Div,
                        .span = Span{start, state.lexer.pos}
                }
        });
        state.lexer.skip_whitespace();
        if (state.lexer.eof()) {
            break;
        }
        c = state.lexer.peek();
    }
    return lhs;
}

std::shared_ptr<Expr> parse_precedence_2(ParserState &state) {
    std::shared_ptr<Expr> lhs = parse_precedence_1(state);
    state.lexer.skip_whitespace();

    if (state.lexer.eof()) {
        return std::move(lhs);
    }

    char c = state.lexer.peek();
    while (c == '+' || c == '-') {
        auto start = state.lexer.pos;
        state.lexer.next();
        std::shared_ptr<Expr> rhs = parse_precedence_1(state);
        lhs = std::make_shared<Expr>(Expr{
                .data = BinaryExpr{
                        .lhs = std::move(lhs),
                        .rhs = std::move(rhs),
                        .op = c == '+' ? Add : Sub,
                        .span = Span{start, state.lexer.pos}
                }
        });
        state.lexer.skip_whitespace();
        if (state.lexer.eof()) {
            break;
        }
        c = state.lexer.peek();
    }
    return lhs;
}

std::shared_ptr<Expr> parse_precedence_3(ParserState &state) {
    std::shared_ptr<Expr> lhs = parse_precedence_2(state);
    state.lexer.skip_whitespace();

    if (state.lexer.eof()) {
        return std::move(lhs);
    }

    char c = state.lexer.peek();
    while (c == '<' || c == '>') {
        auto start = state.lexer.pos;
        state.lexer.next();
        std::shared_ptr<Expr> rhs = parse_precedence_2(state);
        lhs = std::make_shared<Expr>(Expr{
                .data = BinaryExpr{
                        .lhs = std::move(lhs),
                        .rhs = std::move(rhs),
                        .op = c == '<' ? Lt : Gt,
                        .span = Span{start, state.lexer.pos}
                }
        });
        state.lexer.skip_whitespace();
        if (state.lexer.eof()) {
            break;
        }
        c = state.lexer.peek();
    }
    return lhs;
}

std::shared_ptr<Expr> parse_expr(ParserState &state) {
    state.lexer.skip_whitespace();

    if (state.lexer.eof()) {
        throw std::runtime_error("Unexpected end of input");
    }

    return parse_precedence_3(state);
}

StmtList parse_stmt_list(ParserState &state, bool error_on_end = true);

Stmt parse_stmt(ParserState &state, const Name& name) {
    if (name == "if") {
        return Stmt{
                IfStmt{
                        parse_expr(state),
                        std::make_shared<StmtList>(parse_stmt_list(state, false))
                }
        };
    }
    if (name == "while") {
        return Stmt{
            WhileStmt{
                parse_expr(state),
                std::make_shared<StmtList>(parse_stmt_list(state, false))
            }
        };
    }
    state.lexer.skip_whitespace();
    if (state.lexer.eof()) {
        throw std::runtime_error("Unexpected end of input");
    }
    if (state.lexer.next() != '=') {
        throw std::runtime_error("Expected '='");
    }

    return Stmt{
        AssignmentStmt{
            name,
            parse_expr(state),
            Span{name.span.start, state.lexer.pre_ws_pos}
        }
    };
}

StmtList parse_stmt_list(ParserState &state, const bool error_on_end) {
    StmtList stmt_list;

    state.lexer.skip_whitespace();
    while (!state.lexer.eof()) {
        // we are also going to consume the end, or error if error_on_end = true
        Name name = state.lexer.read_name();
        if (name == "end") {
            if (error_on_end) {
                throw std::runtime_error("Unexpected end");
            }
            break;
        }
        stmt_list.statements.push_back(parse_stmt(state, name));
        state.lexer.skip_whitespace();
    }

    return stmt_list;
}

Program parse_program(ParserState &state) {
    Program program;
    program.statements = parse_stmt_list(state, true);
    return program;
}
