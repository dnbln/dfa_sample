#include <algorithm>
#include "parse.hpp"
#include "dfg_analysis.hpp"
#include "dfg.hpp"

std::string SRC = R"(
x = 0

a = 1
if x > 1
    a = 2
end
if x < 2
    a = 3
end

b = a
)";

int main() {
    ParserState state{Lexer{SRC}};
    const Program p = parse_program(state);
    const Cfg cfg = build_cfg(p);
//    dbg_cfg(cfg);
    const Dfg dfg = build_dfg(cfg);
//    dbg_dfg(dfg);
    DfgNodeOutputs outputs;
    compute_whole_program_required_outputs(p, outputs);

    DfgNodeUnusedAssignments unused_assignments;
    analyse_dfg(dfg, outputs, unused_assignments);
    std::ranges::sort(unused_assignments.assignments,
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
