/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/graph-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

#include <jive/util/list.h>

namespace jive {
namespace value {

type::~type() noexcept {}

/* value inputs */

input::~input() noexcept {};

input::input(struct jive_node * node, size_t index, jive_output * initial_operand)
	: jive_input(node, index, initial_operand)
{}

/* value outputs */

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive_output(node, index)
{}

/* value gates */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive_gate(graph, name)
{}

}
}
