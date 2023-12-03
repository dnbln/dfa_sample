#include <algorithm>
#include <fstream>
#include <sstream>

#include "parse.hpp"
#include "dfg_analysis.hpp"
#include "dfg.hpp"

const char* SRC = R"(
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

int main(int argc, char* argv[]) {
    std::string src;
    if (argc > 1) {
        std::ifstream fin(argv[1]);
        if (!fin) {
            std::cerr << "Failed to open file " << argv[1] << std::endl;
            return 1;
        }
        std::stringstream buffer;
        buffer << fin.rdbuf();
        src = buffer.str();
    } else {
        src = SRC;
    }
    ParserState state{Lexer{src}};
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
        std::cout << "Unused assignment to " << assignment->name.name << " at " << assignment->span.start << ".."
                  << assignment->span.end
                  << std::endl
                  << src.substr(assignment->span.start, assignment->span.end - assignment->span.start)
                  << std::endl;
    }
    return 0;
}
