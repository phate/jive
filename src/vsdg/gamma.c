/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma.h>

#include <algorithm>
#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma-normal-form.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>

namespace jive {

gamma_tail_op::~gamma_tail_op() noexcept
{
}
std::string
gamma_tail_op::debug_string() const
{
	return "GAMMA_TAIL";
}

std::unique_ptr<jive::operation>
gamma_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_tail_op(*this));
}

gamma_op::~gamma_op() noexcept
{
}

size_t
gamma_op::narguments() const noexcept
{
	return 1 + nalternatives_;
}

const base::type &
gamma_op::argument_type(size_t index) const noexcept
{
	if (index < nalternatives_) {
		static const achr::type anchor_type;
		return anchor_type;
	} else {
		static const ctl::type control_type(nalternatives_);
		return control_type;
	}
}
std::string
gamma_op::debug_string() const
{
	return "GAMMA";
}

std::unique_ptr<jive::operation>
gamma_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_op(*this));
}

bool
gamma_op::operator==(const operation & other) const noexcept
{
	const gamma_op * op = dynamic_cast<const gamma_op*>(&other);
	return op && op->nalternatives_ == nalternatives_;
}

}

jive::node_normal_form *
jive_gamma_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive::gamma_normal_form * normal_form = new jive::gamma_normal_form(
		operator_class, parent, graph);
	return normal_form;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::gamma_op), jive_gamma_node_get_default_normal_form_);
}


static jive_node *
jive_gamma_create(
	jive::output * predicate,
	const std::vector<const jive::base::type*> & types,
	const std::vector<std::vector<jive::output*>> & alternatives)
{
	size_t nvalues = types.size();
	size_t nalternatives = alternatives.size();

	std::vector<jive::output*> tmp({predicate});
	for (size_t i = 0; i < nalternatives; i++) {
		JIVE_DEBUG_ASSERT(types.size() == alternatives[i].size());
		for (size_t f = 0; f < nvalues; f++)
			tmp.push_back(alternatives[i][f]);
	}
	jive_region * region = jive_region_innermost(tmp.size(), &tmp[0]);

	jive::output * arguments[nalternatives+1];
	for (size_t n = 0; n < nalternatives; n++) {
		jive_region * subregion = jive_region_create_subregion(region);
		jive_node * tail_node = jive::gamma_tail_op().create_node(subregion, 0, nullptr);
		arguments[n] = tail_node->outputs[0];
	}
	arguments[nalternatives] = predicate;

	jive_node * gamma = jive::gamma_op(nalternatives).create_node(region, nalternatives+1, arguments);
	
	for (size_t n = 0; n < nvalues; n++) {
		char name[80];
		snprintf(name, sizeof(name), "gamma_%p_%zd", gamma, n);
		jive::gate * gate = types[n]->create_gate(region->graph, name);
		for (size_t i = 0; i < nalternatives; i++)
			jive_node_gate_input(arguments[i]->node(), gate, alternatives[i][n]);
		jive_node_gate_output(gamma, gate);
	}

	return gamma;
}

std::vector<jive::output *>
jive_gamma(jive::output * predicate,
	const std::vector<const jive::base::type*> & types,
	const std::vector<std::vector<jive::output*>> & alternatives)
{
	/*
		FIXME: This code is supposed to be in gamma-normal-form.c and nowhere else. However,
		we would need to extend the gamma operator with the types vector in order to make
		this possible.
	*/

	if (auto ctl = dynamic_cast<const jive::ctl::type*>(&predicate->type())) {
		if (ctl->nalternatives() != alternatives.size())
			throw jive::compiler_error("Incorrect number of alternatives.");
	}

	for (size_t n = 0; n < alternatives.size(); n++) {
		if (alternatives[n].size() != types.size())
			throw jive::compiler_error("Incorrect number of values for an alternative.");
	}

	/*
		FIXME: This is currently not guarded by any normal form attribute. We could make
		it a special case where two regions are equal, but we have to introduce this
		optimization first.
	*/
	if (alternatives.size() == 1)
		return alternatives[0];

	jive_graph * graph = predicate->node()->region->graph;
	jive::gamma_normal_form * nf = static_cast<jive::gamma_normal_form *>(
		jive_graph_get_nodeclass_form(graph, typeid(jive::gamma_op)));
	
	if (nf->get_mutable() && nf->get_predicate_reduction()) {
		if (auto op = dynamic_cast<const jive::ctl::constant_op*>(&predicate->node()->operation()))
				return alternatives[op->value().alternative()];
	}

	std::vector<jive::output*> results;
	jive_node * node = jive_gamma_create(predicate, types, alternatives);
	for (size_t n = 0; n < node->noutputs; n++)
		results.push_back(node->outputs[n]);

	/*
		FIXME: this algorithm is O(n^2), since we have to iterate through all inputs/outputs
		that have a greater index than the deleted one
	*/
	if (nf->get_mutable() && nf->get_invariant_reduction()) {
		results.clear();
		size_t nalternatives = node->ninputs-1;
		for (size_t v = node->noutputs; v > 0; --v) {
			size_t n;
			jive::output * value = node->producer(0)->inputs[v-1]->origin();
			for (n = 1; n < nalternatives; n++) {
				if (value != node->producer(n)->inputs[v-1]->origin())
					break;
			}
			if (n == nalternatives) {
				results.push_back(node->producer(0)->inputs[v-1]->origin());
				delete node->outputs[v-1];
				for (size_t n = 0; n < nalternatives; n++)
					delete node->producer(n)->inputs[v-1];
			} else
				results.push_back(node->outputs[v-1]);
		}
		std::reverse(results.begin(), results.end());
	}

	return results;
}
