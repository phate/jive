/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma.h>

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
		static const ctl::type control_type;
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
	jive_region * region,
	jive::output * predicate,
	size_t nvalues, const jive::base::type * const types[],
	jive::output * const true_values[],
	jive::output * const false_values[])
{
	jive_region * false_region = jive_region_create_subregion(region);
	jive_region * true_region = jive_region_create_subregion(region);
	jive_node * false_alt = jive::gamma_tail_op().create_node(
		false_region, 0, nullptr);
	jive_node * true_alt = jive::gamma_tail_op().create_node(
		true_region, 0, nullptr);
	jive::output * arguments[3] = {
		true_alt->outputs[0],
		false_alt->outputs[0],
		predicate
	};
	jive_node * gamma = jive::gamma_op(2).create_node(
		region,
		3, arguments);
	
	size_t n;
	for (n = 0; n < nvalues; n++) {
		char name[80];
		snprintf(name, sizeof(name), "gamma_%p_%zd", gamma, n);
		jive::gate * gate = types[n]->create_gate(region->graph, name);
		jive_node_gate_input(true_alt, gate, true_values[n]);
		jive_node_gate_input(false_alt, gate, false_values[n]);
		jive_node_gate_output(gamma, gate);
	}
	return gamma;
}

void
jive_gamma(jive::output * predicate,
	size_t nvalues, const struct jive::base::type * const types[],
	jive::output * const true_values[],
	jive::output * const false_values[],
	jive::output * results[])
{
	size_t n;
	
	jive_graph * graph = predicate->node()->region->graph;
	jive::gamma_normal_form * nf = static_cast<jive::gamma_normal_form *>(
		jive_graph_get_nodeclass_form(graph, typeid(jive::gamma_op)));
	
	if (nf->get_mutable() && nf->get_predicate_reduction()) {
		const jive::ctl::constant_op * op =
			dynamic_cast<const jive::ctl::constant_op *>(&predicate->node()->operation());
		if (op && op->value()) {
			for (n = 0; n < nvalues; ++n)
				results[n] = true_values[n];
			return;
		} else if (op && !op->value()) {
			for (n = 0; n < nvalues; ++n)
				results[n] = false_values[n];
			return;
		}
	}
	
	jive::output * tmp[nvalues * 2 + 1];
	tmp[0] = predicate;
	for (n = 0; n < nvalues; n++)
		tmp[n + 1] = false_values[n];
	for (n = 0; n < nvalues; n++)
		tmp[n + nvalues + 1] = true_values[n];
	jive_region * region = jive_region_innermost(nvalues * 2 + 1, tmp);
	
	jive_node * node = jive_gamma_create(region, predicate, nvalues, types, true_values, false_values);
	
	for (n = 0; n < nvalues; ++n)
		results[n] = node->outputs[n];
	
	if (nf->get_mutable() && nf->get_invariant_reduction()) {
		jive_node * true_branch = node->producer(0);
		jive_node * false_branch = node->producer(1);
		for (n = nvalues; n > 0; --n) {
			if (true_values[n-1] != false_values[n-1])
				continue;
			results[n-1] = true_values[n-1];
			delete node->outputs[n-1];
			delete true_branch->inputs[n-1];
			delete false_branch->inputs[n-1];
		}
	}
}
