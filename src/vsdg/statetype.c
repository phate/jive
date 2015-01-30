/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
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

	return jive_node_create_normalized(
		states[0]->node()->region->graph,
		op,
		std::vector<jive::output *>(states, states + nstates))[0];
}

std::vector<jive::output *>
jive_state_split(const jive::state::type * statetype, jive::output * state, size_t nstates)
{
	jive::state::mux_op op(*statetype, 1, nstates);

	return jive_node_create_normalized(
		state->node()->region->graph,
		op,
		{state});
}
