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

const jive_node_class JIVE_STATEMUX_NODE = {
	parent : &JIVE_NODE,
	name : "STATEMUX",
	fini : jive_node_fini_,
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

namespace jive {
namespace state {

type::~type() noexcept {}

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index, jive::output * origin)
	: jive::input(node, index, origin)
{}

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive::output(node, index)
{}

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::gate(graph, name)
{}

mux_op::~mux_op() noexcept {}

bool
mux_op::operator==(const operation & other) const noexcept
{
	const mux_op * op = dynamic_cast<const mux_op *>(&other);
	return op &&
		op->narguments_ == narguments_ &&
		op->nresults_ == nresults_ &&
		*op->state_type_ == *state_type_;
}

size_t
mux_op::narguments() const noexcept
{
	return narguments_;
}

const jive::base::type &
mux_op::argument_type(size_t index) const noexcept
{
	return *state_type_;
}

size_t
mux_op::nresults() const noexcept
{
	return nresults_;
}

const jive::base::type &
mux_op::result_type(size_t index) const noexcept
{
	return *state_type_;
}

jive_node *
mux_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, &JIVE_STATEMUX_NODE, region, arguments, arguments + narguments);
}

std::string
mux_op::debug_string() const
{
	return "STATEMUX";
}

std::unique_ptr<jive::operation>
mux_op::copy() const
{
	return std::unique_ptr<jive::operation>(new mux_op(*this));
}

}
}

jive::output *
jive_state_merge(const jive::state::type * statetype, size_t nstates, jive::output * const states[])
{
	jive::state::mux_op op(*statetype, nstates, 1);

	jive::output * result;
	jive_node_create_normalized(&JIVE_STATEMUX_NODE, states[0]->node()->region->graph,
		&op, nstates, states, &result);
	return result;
}

std::vector<jive::output *>
jive_state_split(const jive::state::type * statetype, jive::output * state, size_t nstates)
{
	jive::state::mux_op op(*statetype, 1, nstates);

	jive::output * results[nstates];
	jive_node_create_normalized(&JIVE_STATEMUX_NODE, state->node()->region->graph,
		&op, 1, &state, results);

	return std::vector<jive::output *>(results, results + nstates);
}
