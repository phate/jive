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

jive_value_type::~jive_value_type() noexcept {}

/* value inputs */

jive_value_input::~jive_value_input() noexcept {};

jive_value_input::jive_value_input(struct jive_node * node, size_t index,
	jive_output * initial_operand)
	: jive_input(node, index, initial_operand)
{}

/* value outputs */

jive_value_output::~jive_value_output() noexcept {}

jive_value_output::jive_value_output(struct jive_node * node, size_t index)
	: jive_output(node, index)
{}

/* value gates */

jive_value_gate::~jive_value_gate() noexcept {}

jive_value_gate::jive_value_gate(jive_graph * graph, const char name[])
	: jive_gate(graph, name)
{}
