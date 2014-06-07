/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/statetype.h>

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace state {

type::~type() noexcept {}

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive::input(node, index, origin)
{}

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive_output(node, index)
{}

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive_gate(graph, name)
{}

}
}












static void
jive_statemux_node_fini_(jive_node * self_);

static bool
jive_statemux_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_statemux_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_STATEMUX_NODE = {
	parent : &JIVE_NODE,
	name : "STATEMUX",
	fini : jive_statemux_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_statemux_node_match_attrs_, /* override */
	check_operands : NULL,
	create : jive_statemux_node_create_, /* override */
};

static void
jive_statemux_node_fini_(jive_node * self_)
{
	jive_node_fini_(self_);
}

static bool
jive_statemux_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_statemux_node * self = (const jive_statemux_node *) self_;
	const jive::statemux_operation * attrs = (const jive::statemux_operation *) attrs_;
	return self->operation().type() == attrs->type();
}

static jive_node *
jive_statemux_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive::statemux_operation * attrs = (const jive::statemux_operation *) attrs_;
	return jive_statemux_node_create(region, &attrs->type(), noperands, operands, attrs->noutputs());
}

jive_node *
jive_statemux_node_create(jive_region * region,
	const jive::base::type * statetype,
	size_t noperands, jive_output * const operands[],
	size_t noutputs)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::state::type*>(statetype));
	jive_context * context = region->graph->context;
	jive_statemux_node * node = new jive_statemux_node(
		jive::statemux_operation(noutputs, *statetype));
	
	node->class_ = &JIVE_STATEMUX_NODE;
	
	const jive::base::type * operand_types[noperands];
	const jive::base::type * output_types[noutputs];
	size_t n;
	for (n = 0; n < noperands; n++)
		operand_types[n] = statetype;
	for (n = 0; n < noutputs; n++)
		output_types[n] = statetype;
		
	jive_node_init_(node, region,
		noperands, operand_types, operands,
		noutputs, output_types);
	
	return node;
}

jive_output *
jive_state_merge(const jive::base::type * statetype, size_t nstates, jive_output * const states[])
{
	jive_region * region = jive_region_innermost(nstates, states);
	return jive_statemux_node_create(region, statetype, nstates, states, 1)->outputs[0];
}

jive_node *
jive_state_split(const jive::base::type * statetype, jive_output * state, size_t nstates)
{
	jive_region * region = state->node()->region;
	return jive_statemux_node_create(region, statetype, 1, &state, nstates);
}
