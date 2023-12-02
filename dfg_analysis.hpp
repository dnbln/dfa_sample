//
// Created by Dinu on 12/2/2023.
//

#ifndef DFA_SAMPLE_DFG_ANALYSIS_HPP
#define DFA_SAMPLE_DFG_ANALYSIS_HPP

#include <map>
#include <ranges>
#include <utility>
#include "ast.hpp"
#include "visitor.hpp"
#include "cfg.hpp"
#include "dfg.hpp"

struct DfgNodeUnusedAssignments {
    std::vector<std::shared_ptr<AssignmentCfgNode>> assignments;
};

struct DfgNodeInputs {
    std::set<char> in;
};

struct DfgNodeOutputs {
    std::set<char> out;
};

struct DfgNodeInout {
    DfgNodeInputs inputs;
    DfgNodeOutputs outputs;
};

void analyse_dfg(Dfg& dfg, const DfgNodeOutputs& whole_program_outputs, DfgNodeUnusedAssignments& unused_assignments);
void compute_whole_program_required_outputs(const Program& program, DfgNodeOutputs& whole_program_outputs);

#endif //DFA_SAMPLE_DFG_ANALYSIS_HPP
