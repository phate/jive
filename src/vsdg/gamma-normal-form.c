/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
jive_move_context_append(jive_move_context * self, jive_node * node)
{
	if (node->depth_from_root >= self->depths.size())
		self->depths.resize(node->depth_from_root+1);

	self->depths[node->depth_from_root].push_back(node);
}

static void
pre_move_region(jive_region * target_region, const jive_region * original_region,
	jive_move_context * move_context)
{
	jive_node * node;
	JIVE_LIST_ITERATE(original_region->nodes, node, region_nodes_list) {
		if (node != original_region->bottom)
			jive_move_context_append(move_context, node);
	}
}

static void
jive_region_move(const jive_region * self, jive_region * target)
{
	jive_move_context move_context;

	pre_move_region(target, self, &move_context);

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
	if (!get_mutable())
		return true;

	bool was_normalized = true;
	if (get_predicate_reduction()) {
		jive::output * predicate = node->inputs[node->ninputs-1]->origin();
		if (auto op = dynamic_cast<const jive::ctl::constant_op*>(&predicate->node()->operation())) {
			jive_node * branch = node->producer(op->value().nalternatives());
			jive_region_move(branch->region, node->region);
			for (size_t n = 0; n < node->noutputs; n++)
				node->outputs[n]->replace(branch->inputs[n]->origin());
			was_normalized = false;
		}
	}

	if (get_invariant_reduction()) {
		size_t nalternatives = node->ninputs-1;
		for (size_t v = node->noutputs; v > 0; --v) {
			size_t n;
			jive::output * value = node->producer(0)->inputs[v-1]->origin();
			for (n = 1; n < nalternatives; n++) {
				if (value != node->producer(n)->inputs[v-1]->origin())
					break;
			}
			if (n == nalternatives) {
				node->outputs[v-1]->replace(node->producer(0)->inputs[v-1]->origin());
				delete node->outputs[v-1];
				for (size_t n = 0; n < nalternatives; n++)
					delete node->producer(n)->inputs[v-1];
				was_normalized = false;
			}
		}
	}

	return was_normalized;
}

bool
gamma_normal_form::operands_are_normalized(
	const jive::operation & op,
	const std::vector<jive::output *> & arguments) const
{
	if (!get_mutable())
		return true;
	
	if (get_predicate_reduction()) {
		jive::output * predicate = arguments[arguments.size()-1];
		if (dynamic_cast<const jive::ctl::constant_op *>(&predicate->node()->operation()))
			return false;
	}
	
	if (get_invariant_reduction()) {
		size_t nalternatives = arguments.size()-1;
		for (size_t v = 0; v < arguments[0]->node()->ninputs; v++) {
			size_t n;
			jive::output * value = arguments[0]->node()->inputs[v]->origin();
			for (n = 1; n < nalternatives; n++) {
				if (value != arguments[n]->node()->inputs[v]->origin())
					break;
			}
			if (n == nalternatives)
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
	jive_region * region,
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
