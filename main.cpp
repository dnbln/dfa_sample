#include <algorithm>
#include "parse.hpp"
#include "dfg_analysis.hpp"
#include "dfg.hpp"

std::string SRC = R"(
a = 1 + 2 * 3 + 4 * 5 > 6 * 7 + 8 * 9 * 10 / 11 > 12 * 13 + 14 / 15 * 16
)";

int main() {
    ParserState state{Lexer{SRC}};
    Program p = parse_program(state);
    Cfg cfg = build_cfg(p);
    dbg_cfg(cfg);
    Dfg dfg = build_dfg(cfg);
//    dbg_dfg(dfg);
    DfgNodeOutputs outputs;
    compute_whole_program_required_outputs(p, outputs);

    DfgNodeUnusedAssignments unused_assignments;
    analyse_dfg(dfg, outputs, unused_assignments);
    std::sort(unused_assignments.assignments.begin(), unused_assignments.assignments.end(),
              [](const auto &a, const auto &b) {
                  return a->span.start < b->span.start;
              });
    for (const auto &assignment: unused_assignments.assignments) {
        std::cout << "Unused assignment " << assignment->name.name << " at " << assignment->span.start << ".."
                  << assignment->span.end
                  << std::endl
                  << SRC.substr(assignment->span.start, assignment->span.end - assignment->span.start)
                  << std::endl;
    }
    return 0;
}
