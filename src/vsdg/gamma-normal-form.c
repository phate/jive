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
#include <jive/vsdg/substitution.h>

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
		jive::output * predicate = node->input(node->ninputs()-1)->origin();
		if (auto op = dynamic_cast<const jive::ctl::constant_op*>(&predicate->node()->operation())) {
			jive_node * tail = node->producer(op->value().nalternatives());
			jive_node * head = tail->input(0)->origin()->node();
			JIVE_DEBUG_ASSERT(tail = tail->region()->bottom);
			JIVE_DEBUG_ASSERT(head = head->region()->top);

			jive::substitution_map map;
			for (size_t n = 1; n < head->noutputs; n++)
				map.insert(head->outputs[n], head->input(n-1)->origin());

			jive_region_copy_substitute(tail->region(), node->region(), map, false, false);

			for (size_t n = 1; n < node->noutputs; n++) {
					jive::output * original = tail->input(n)->origin();
					node->outputs[n]->replace(map.lookup(original));
			}
			was_normalized = false;
		}
	}

	if (get_invariant_reduction()) {
		size_t nalternatives = node->ninputs()-1;
		for (size_t v = node->noutputs; v > 0; --v) {
			size_t n;
			jive::output * value = node->producer(0)->input(v-1)->origin();
			for (n = 1; n < nalternatives; n++) {
				if (value != node->producer(n)->input(v-1)->origin())
					break;
			}
			if (n == nalternatives) {
				node->outputs[v-1]->replace(node->producer(0)->input(v-1)->origin());
				delete node->outputs[v-1];
				for (size_t n = 0; n < nalternatives; n++)
					node->producer(n)->remove_input(v-1);
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
		for (size_t v = 0; v < arguments[0]->node()->ninputs(); v++) {
			size_t n;
			jive::output * value = arguments[0]->node()->input(v)->origin();
			for (n = 1; n < nalternatives; n++) {
				if (value != arguments[n]->node()->input(v)->origin())
					break;
			}
			if (n == nalternatives)
				return false;
		}
	}

	if (get_invariant_reduction()) {
		jive_node * true_branch = arguments[0]->node();
		jive_node * false_branch = arguments[1]->node();
		size_t nvars = true_branch->ninputs();
		for (size_t n = nvars; n > 0; --n) {
			if (true_branch->input(n-1)->origin() == false_branch->input(n-1)->origin()) {
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
