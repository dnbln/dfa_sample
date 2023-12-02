#include "parse.hpp"
#include "dfg_analysis.hpp"
#include "dfg.hpp"

std::string SRC = R"(
a = 1
b = a
x = 3
y = 4

while (b < 5)
  z = x
  b = b + 1
  x = 9
  y = 10
end
)";

int main() {
    ParserState state{Lexer{SRC}};
    Program p = parse_program(state);
    Cfg cfg = build_cfg(p);
//    dbg_cfg(cfg);
    Dfg dfg = build_dfg(cfg);
//    dbg_dfg(dfg);
    DfgNodeOutputs outputs;
    compute_whole_program_required_outputs(p, outputs);

    DfgNodeUnusedAssignments unused_assignments;
    analyse_dfg(dfg, outputs, unused_assignments);
    for (const auto &assignment: unused_assignments.assignments) {
        std::cout << "Unused assignment " << assignment->name.name << " at " << assignment->span.start << ".."
                  << assignment->span.end
                  << std::endl
                  << SRC.substr(assignment->span.start, assignment->span.end - assignment->span.start)
                  << std::endl;
    }
    return 0;
}
