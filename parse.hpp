//
// Created by Dinu on 12/2/2023.
//

#ifndef DFA_SAMPLE_PARSE_HPP
#define DFA_SAMPLE_PARSE_HPP

#include "ast.hpp"

struct Lexer {
    std::string_view input;
    size_t pos = 0;
    size_t pre_ws_pos = 0;

    char peek() {
        return input[pos];
    }

    char next() {
        return input[pos++];
    }

    bool eof() {
        return pos >= input.size();
    }

    void skip_whitespace() {
        pre_ws_pos = pos;
        while (!eof() && isspace(peek())) {
            next();
        }
    }

    Name read_name() {
        skip_whitespace();

        auto start = pos;
        while (!eof() && isalnum(peek())) {
            next();
        }

        return Name{
                input.substr(start, pos - start),
                Span{start, pos}
        };
    }

    Constant read_number() {
        skip_whitespace();

        auto start = pos;
        while (!eof() && isdigit(peek())) {
            next();
        }
        int val = std::stoi(std::string(input.substr(start, pos - start)));
        return Constant{
                val,
                Span{start, pos}
        };
    }
};

struct ParserState {
    Lexer lexer;
};

Program parse_program(ParserState &state);

#endif //DFA_SAMPLE_PARSE_HPP
