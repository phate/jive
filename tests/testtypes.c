/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testtypes.h"

#include <jive/util/buffer.h>

/* test value type */

jive_test_value_type::~jive_test_value_type() noexcept {}

void
jive_test_value_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "test_value");
}

bool
jive_test_value_type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive_test_value_type*>(&other) != nullptr;
}

jive_test_value_type *
jive_test_value_type::copy() const
{
	return new jive_test_value_type();
}

jive::input *
jive_test_value_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_test_value_input(node, index, origin);
}

jive_output *
jive_test_value_type::create_output(jive_node * node, size_t index) const
{
	return new jive_test_value_output(node, index);
}

jive_gate *
jive_test_value_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_test_value_gate(graph, name);
}

jive_test_value_input::~jive_test_value_input() noexcept {}

jive_test_value_input::jive_test_value_input(jive_node * node, size_t index,
	jive_output * origin)
	: jive::value::input(node, index, origin)
{}

jive_test_value_output::~jive_test_value_output() noexcept {}

jive_test_value_output::jive_test_value_output(jive_node * node, size_t index)
	: jive::value::output(node, index)
{}

jive_test_value_gate::~jive_test_value_gate() noexcept {}

jive_test_value_gate::jive_test_value_gate(jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
{}

/* test state type */

jive_test_state_type::~jive_test_state_type() noexcept {}

void
jive_test_state_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "test_state");
}

bool
jive_test_state_type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive_test_state_type*>(&other) != nullptr;
}

jive_test_state_type *
jive_test_state_type::copy() const
{
	return new jive_test_state_type();
}

jive::input *
jive_test_state_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_test_state_input(node, index, origin);
}

jive_output *
jive_test_state_type::create_output(jive_node * node, size_t index) const
{
	return new jive_test_state_output(node, index);
}

jive_gate *
jive_test_state_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_test_state_gate(graph, name);
}

jive_test_state_input::~jive_test_state_input() noexcept {}

jive_test_state_input::jive_test_state_input(jive_node * node, size_t index,
	jive_output * origin)
	: jive::state::input(node, index, origin)
{}

jive_test_state_output::~jive_test_state_output() noexcept {}

jive_test_state_output::jive_test_state_output(jive_node * node, size_t index)
	: jive::state::output(node, index)
{}

jive_test_state_gate::~jive_test_state_gate() noexcept {}

jive_test_state_gate::jive_test_state_gate(jive_graph * graph, const char name[])
	: jive::state::gate(graph, name)
{}
