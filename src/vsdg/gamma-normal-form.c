/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma-normal-form.h>

#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

namespace {

typedef struct jive_move_context jive_move_context;
struct jive_move_context {
	std::vector<std::vector<jive_node*>> depths;
};

static void
jive_move_context_append(jive_move_context * self, jive_context * context, jive_node * node)
{
	if (node->depth_from_root >= self->depths.size())
		self->depths.resize(node->depth_from_root+1);

	self->depths[node->depth_from_root].push_back(node);
}

static void
pre_move_region(jive_region * target_region, const jive_region * original_region,
	jive_move_context * move_context, jive_context * context)
{
	jive_node * node;
	JIVE_LIST_ITERATE(original_region->nodes, node, region_nodes_list) {
		if (node != original_region->bottom)
			jive_move_context_append(move_context, context, node);
	}
}

static void
jive_region_move(const jive_region * self, jive_region * target)
{
	jive_context * context = target->graph->context;
	jive_move_context move_context;

	pre_move_region(target, self, &move_context, context);

	for (size_t depth = 0; depth < move_context.depths.size(); depth++) {
		for (size_t n = 0; n < move_context.depths[depth].size(); n++) {
			jive_node * node = move_context.depths[depth][n];
			jive_node_move(node, target);
		}
	}
}

}

namespace jive {

gamma_normal_form::~gamma_normal_form() noexcept
{
}

gamma_normal_form::gamma_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph) noexcept
	: anchor_normal_form(operator_class, parent, graph)
	, enable_predicate_reduction_(true)
	, enable_invariant_reduction_(true)
{
	if (auto p = dynamic_cast<gamma_normal_form *>(parent)) {
		enable_predicate_reduction_ = p->enable_predicate_reduction_;
		enable_invariant_reduction_ = p->enable_invariant_reduction_;
	}
}

bool
gamma_normal_form::normalize_node(jive_node * node) const
{
	bool was_normalized = true;

	if (!get_mutable()) {
		return true;
	}
	
	JIVE_DEBUG_ASSERT(node->noperands == 3);
	
	if (get_predicate_reduction()) {
		jive::output * pred = node->inputs[2]->origin();
		jive_node * branch = nullptr;
		const jive::ctl::constant_op * op =
			dynamic_cast<const jive::ctl::constant_op *>(&pred->node()->operation());
		if (op && op->value() == true) {
			branch = node->producer(0);
		} else if (op && op->value() == false) {
			branch = node->producer(1);
		}
		
		if (branch) {
			jive_region_move(branch->region, node->region);
			for (size_t n = 0; n < node->noutputs; n++) {
				jive_output_replace(node->outputs[n], branch->inputs[n]->origin());
			}

			was_normalized = false;
		}
	}
	
	if (get_invariant_reduction()) {
		jive_node * true_branch = node->producer(0);
		jive_node * false_branch = node->producer(1);
		for (size_t n = node->noutputs; n > 0; --n) {
			if (true_branch->inputs[n-1]->origin() != false_branch->inputs[n-1]->origin()) {
				continue;
			}
			jive_output_replace(node->outputs[n-1], true_branch->inputs[n-1]->origin());
			delete node->outputs[n-1];
			delete true_branch->inputs[n-1];
			delete false_branch->inputs[n-1];
			was_normalized = false;
		}
	}
	
	return was_normalized;
}

bool
gamma_normal_form::operands_are_normalized(
	const jive::operation & op,
	const std::vector<jive::output *> & arguments) const
{
	if (!get_mutable()) {
		return true;
	}
	
	JIVE_DEBUG_ASSERT(arguments.size() == 3);
	
	if (get_predicate_reduction()) {
		jive::output * pred = arguments[2];
		jive_node * branch = nullptr;
		const jive::ctl::constant_op * op =
			dynamic_cast<const jive::ctl::constant_op *>(&pred->node()->operation());
		if (op) {
			return false;
		}
	}
	
	if (get_invariant_reduction()) {
		jive_node * true_branch = arguments[0]->node();
		jive_node * false_branch = arguments[1]->node();
		size_t nvars = true_branch->ninputs;
		for (size_t n = nvars; n > 0; --n) {
			if (true_branch->inputs[n-1]->origin() == false_branch->inputs[n-1]->origin()) {
				return false;
			}
		}
	}

	return true;
}

std::vector<jive::output *>
gamma_normal_form::normalized_create(
	const jive::operation & op,
	const std::vector<jive::output *> & arguments) const
{
	throw std::logic_error("Unimplemented: gamma_normal_form::normalized_create");
}

void
gamma_normal_form::set_reducible(bool enable)
{
	anchor_normal_form::set_reducible(enable);
	set_predicate_reduction(enable);
	set_invariant_reduction(enable);
}

void
gamma_normal_form::set_predicate_reduction(bool enable)
{
	if (enable_predicate_reduction_ == enable) {
		return;
	}
	
	children_set<gamma_normal_form, &gamma_normal_form::set_predicate_reduction>(enable);

	enable_predicate_reduction_ = enable;

	if (enable && get_mutable()) {
		jive_graph_mark_denormalized(graph());
	}
}

void
gamma_normal_form::set_invariant_reduction(bool enable)
{
	if (enable_invariant_reduction_ == enable) {
		return;
	}

	children_set<gamma_normal_form, &gamma_normal_form::set_invariant_reduction>(enable);

	enable_invariant_reduction_ = enable;

	if (enable && get_mutable()) {
		jive_graph_mark_denormalized(graph());
	}
}

}
