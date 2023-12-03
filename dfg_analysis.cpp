//
// Created by Dinu on 12/2/2023.
//

#include "dfg_analysis.hpp"

struct InputsReadVisitor : public AstVisitor {
    std::set<char> inputs;

    void visit_name(const Name &name) override {
        inputs.insert(name.name[0]);
    }
};


struct AnalyseDfgContext {
    const Dfg &dfg;
    DfgNodeOutputs outputs;
    DfgNodeUnusedAssignments &unused_assignments;
    std::vector<std::shared_ptr<DfgNode>> &work_list;
    std::set<DfgNode *> &visited;
    std::map<DfgNode *, DfgNodeInout> &inouts;
};

static void analyse_dfg_impl(AnalyseDfgContext &context);

static DfgNodeInputs compute_dfg_node_inputs(const BasicCfgBlock &block, const DfgNodeOutputs &outputs,
                                             DfgNodeUnusedAssignments &unused_assignments) {
    DfgNodeOutputs required_outputs = outputs;
    for (const auto &assignment: std::ranges::reverse_view(block.assignments)) {
        if (assignment->name.name.length() != 1) {
            throw std::runtime_error("Invalid name length");
        }

        char name = assignment->name.name[0];
        if (required_outputs.out.contains(name)) {
            required_outputs.out.erase(name);
        } else {
            bool push = true;
            for (const auto &unused_assignment: unused_assignments.assignments) {
                if (unused_assignment.get() == assignment.get()) {
                    push = false;
                    break;
                }
            }
            if (push) {
                unused_assignments.assignments.push_back(assignment);
            }
        }

        InputsReadVisitor visitor;
        visitor.visit_expr(*assignment->expr);
        required_outputs.out.insert(visitor.inputs.begin(), visitor.inputs.end());
    }

    DfgNodeInputs inputs;
    inputs.in = required_outputs.out;
    return inputs;
}

static std::shared_ptr<DfgNode>
dfg_ptr_for_while_end_dummy_node(const Dfg &dfg, const std::shared_ptr<CfgNode> &while_node) {
    for (const auto &node: dfg.nodes) {
        bool result = std::visit(
                [&](auto &&node) -> bool {
                    using T = std::decay_t<decltype(node)>;
                    if constexpr (std::is_same_v<T, std::shared_ptr<WhileRetDummyCfgNode>>) {
                        auto v = node->while_node.value().lock();
                        return v.get() == while_node.get();
                    } else {
                        return false;
                    }
                }, node->cfg_node->node);
        if (result) {
            return node;
        }
    }
    throw std::runtime_error("Could not find dfg node for while end dummy node");
}

void dbg_inouts(const std::shared_ptr<DfgNode> &node, const DfgNodeInout &inout) {
    std::cout << "Node: \n";
    dbg_cfg_node(*node->cfg_node);
    std::cout << "Inputs: ";
    for (const auto &input: inout.inputs.in) {
        std::cout << input << " ";
    }
    std::cout << std::endl;
    std::cout << "Outputs: ";
    for (const auto &output: inout.outputs.out) {
        std::cout << output << " ";
    }
    std::cout << std::endl;
}


static DfgNodeInputs
compute_dfg_node_inputs_for_while(const std::shared_ptr<WhileCfgNode> &while_node,
                                  const std::shared_ptr<CfgNode> &while_cfg_node, AnalyseDfgContext &context) {
    std::set<DfgNode *> vis = context.visited;
    std::shared_ptr<DfgNode> end_node = dfg_ptr_for_while_end_dummy_node(context.dfg, while_cfg_node);
    std::vector<std::shared_ptr<DfgNode>> init_wl;
    for (const auto &in_node: end_node->in_nodes) {
        init_wl.push_back(in_node.lock());
    }
    std::vector<std::shared_ptr<DfgNode>> wl = init_wl;

    AnalyseDfgContext new_context = AnalyseDfgContext{
            .dfg = context.dfg,
            .outputs = context.outputs,
            .unused_assignments = context.unused_assignments,
            .work_list = wl,
            .visited = vis,
            .inouts = context.inouts,
    };
    DfgNodeInputs required_inputs = DfgNodeInputs{.in = context.outputs.out};
    InputsReadVisitor visitor;
    visitor.visit_expr(*while_node->condition);
    required_inputs.in.insert(visitor.inputs.begin(), visitor.inputs.end());
    new_context.inouts.insert_or_assign(end_node.get(),
                                        DfgNodeInout{.inputs = required_inputs, .outputs = context.outputs});

//    std::cout << "Before:\n";
//    dbg_inouts(end_node, (*new_context.inouts)[end_node.get()]);

    analyse_dfg_impl(new_context);
    auto local = new_context.inouts[dfg_ptr_for_cfg(context.dfg, *while_node->body).get()];
    wl = init_wl;
    local.inputs.in.insert(visitor.inputs.begin(), visitor.inputs.end());
    vis = context.visited;
    new_context.inouts.insert_or_assign(end_node.get(), local);
//    std::cout << "Before2:\n";
//    dbg_inouts(end_node, (*new_context.inouts)[end_node.get()]);
    analyse_dfg_impl(new_context);
    local = new_context.inouts[dfg_ptr_for_cfg(context.dfg, *while_node->body).get()];
    local.inputs.in.insert(visitor.inputs.begin(), visitor.inputs.end());
//    std::cout << "After:\n";
//    for (const auto &c: local.inputs.in) {
//        std::cout << c << " ";
//    }
//    std::cout << std::endl;
    return local.inputs;
}

static DfgNodeInputs
compute_dfg_node_inputs_for_if(const IfCfgNode &if_node, const DfgNodeOutputs &outputs) {
    DfgNodeInputs required_inputs = DfgNodeInputs{.in = outputs.out};
    InputsReadVisitor visitor;
    visitor.visit_expr(*if_node.condition);
    required_inputs.in.insert(visitor.inputs.begin(), visitor.inputs.end());
    return required_inputs;
}


static std::shared_ptr<DfgNode> find_end_node(Dfg &dfg) {
    // the end node is likelier to be at the end of the list
    for (const auto &node: std::ranges::reverse_view(dfg.nodes)) {
        if (std::holds_alternative<ExitCfgNode>(node->cfg_node->node)) {
            return node;
        }
    }
    throw std::runtime_error("Could not find end node");
}

void compute_whole_program_required_outputs(const Program &program, DfgNodeOutputs &whole_program_outputs) {
    InputsReadVisitor visitor;
    visitor.visit_program(program);
    whole_program_outputs.out = visitor.inputs;
}

static std::shared_ptr<DfgNode>
next_work_list_node(std::vector<std::shared_ptr<DfgNode>> &work_list, const std::set<DfgNode *> &visited) {
    if (work_list.empty()) {
        throw std::runtime_error("Work list empty");
    }

    for (int i = 0; i < work_list.size(); i++) {
        std::shared_ptr<DfgNode> node_candidate = work_list[i];

        bool good = true;
        for (const auto &out_node: node_candidate->out_nodes) {
            if (!visited.contains(out_node.lock().get())) {
                good = false;
            }
        }
        if (good) {
            work_list.erase(work_list.begin() + i);
            return node_candidate;
        }
    }

//    throw std::runtime_error("Could not find next work list node. Does the DFG contain cycles?");
    auto candidate = work_list.back();
    work_list.pop_back();
    return candidate;
}

static void analyse_dfg_impl(AnalyseDfgContext &context) {
    while (!context.work_list.empty()) {
        std::shared_ptr<DfgNode> node = next_work_list_node(context.work_list, context.visited);

        if (context.visited.contains(node.get())) {
            continue;
        }

        context.visited.insert(node.get());

        DfgNodeOutputs required_outputs;
        for (const auto &out_node: node->out_nodes) {
            auto it = context.inouts[out_node.lock().get()];
            required_outputs.out.insert(it.inputs.in.begin(), it.inputs.in.end());
        }

        bool skip_in_nodes = false;

        std::visit([&](auto &&cfg_node) {
            using T = std::decay_t<decltype(cfg_node)>;
            if constexpr (std::is_same_v<T, BasicCfgBlock>) {
                DfgNodeInputs inputs = compute_dfg_node_inputs(cfg_node, required_outputs,
                                                               context.unused_assignments);
                context.inouts.insert_or_assign(node.get(),
                                                DfgNodeInout{.inputs = inputs, .outputs = required_outputs});
            } else if constexpr (std::is_same_v<T, std::shared_ptr<WhileCfgNode>>) {
                AnalyseDfgContext while_context = context;
                while_context.outputs = required_outputs;
                DfgNodeInputs inputs = compute_dfg_node_inputs_for_while(
                        cfg_node,
                        node->cfg_node,
                        context);
                context.inouts.insert_or_assign(node.get(),
                                                DfgNodeInout{.inputs = inputs, .outputs = required_outputs});
                auto end = dfg_ptr_for_while_end_dummy_node(context.dfg, node->cfg_node);
                for (const auto &in_node: node->in_nodes) {
                    auto locked = in_node.lock();
                    if (locked.get() == end.get()) {
                        continue;
                    }
                    context.work_list.push_back(locked);
                }
                skip_in_nodes = true;
            } else if constexpr (std::is_same_v<T, IfCfgNode>) {
                DfgNodeInputs inputs = compute_dfg_node_inputs_for_if(
                        cfg_node,
                        required_outputs);
                context.inouts.insert_or_assign(node.get(),
                                                DfgNodeInout{.inputs = inputs, .outputs = required_outputs});
            } else {
                context.inouts.insert_or_assign(node.get(),
                                                DfgNodeInout{.inputs = DfgNodeInputs{.in = required_outputs.out}, .outputs = required_outputs});
            }
        }, node->cfg_node->node);

//        dbg_inouts(node, (*context.inouts)[node.get()]);

        if (!skip_in_nodes) {
            for (const auto &in_node: node->in_nodes) {
                context.work_list.push_back(in_node.lock());
            }
        }
    }
}

void analyse_dfg(Dfg &dfg, const DfgNodeOutputs &whole_program_outputs, DfgNodeUnusedAssignments &unused_assignments) {
    std::shared_ptr<DfgNode> end_node = find_end_node(dfg);
    std::vector<std::shared_ptr<DfgNode>> work_list;
    std::set<DfgNode *> visited;
    std::map<DfgNode *, DfgNodeInout> inouts;

    for (const auto &in_node: end_node->in_nodes) {
        work_list.push_back(in_node.lock());
    }
    inouts.insert({end_node.get(), DfgNodeInout{
            .inputs = DfgNodeInputs{.in = whole_program_outputs.out},
            .outputs = DfgNodeOutputs(),
    }});

    auto context = AnalyseDfgContext{
            .dfg = dfg,
            .outputs = whole_program_outputs,
            .unused_assignments = unused_assignments,
            .work_list = work_list,
            .visited = visited,
            .inouts = inouts,
    };

    analyse_dfg_impl(context);
}