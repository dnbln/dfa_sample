cmake_minimum_required(VERSION 3.26)
project(dfa_sample)

set(CMAKE_CXX_STANDARD 23)

add_executable(dfa_sample main.cpp
        ast.hpp
        visitor.hpp
        parse.hpp
        dfg_analysis.hpp
        cfg.hpp
        dfg.hpp
        dfg.cpp
        dfg_analysis.cpp
        visitor.cpp
        cfg.cpp
        parse.cpp)
