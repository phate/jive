/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testtypes.h"

#include <jive/util/buffer.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/valuetype-private.h>

/* test value type */

static jive_input *
jive_test_value_type_create_input_(const jive_type * self, jive_node * node, size_t index,
	jive_output * origin);
static jive_output *
jive_test_value_type_create_output_(const jive_type * self, jive_node * node, size_t index);
static jive_gate *
jive_test_value_type_create_gate_(const jive_type * self, jive_graph * graph, const char name[]);
static jive_type *
jive_test_value_type_copy_(const jive_type * self);

const jive_type_class JIVE_TEST_VALUE_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "test_value",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_test_value_type_create_input_, /* override */
	create_output : jive_test_value_type_create_output_, /* override */
	create_gate : jive_test_value_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_test_value_type_copy_, /* override */
};

jive_test_value_type::~jive_test_value_type() noexcept {}

jive_test_value_type::jive_test_value_type() noexcept
	: jive_value_type(&JIVE_TEST_VALUE_TYPE)
{}

void
jive_test_value_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "test_value");
}

bool
jive_test_value_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_test_value_type*>(&other) != nullptr;
}

std::unique_ptr<jive_type>
jive_test_value_type::copy() const
{
	return std::unique_ptr<jive_test_value_type>(new jive_test_value_type());
}

jive_input *
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

static jive_type *
jive_test_value_type_copy_(const jive_type * self_)
{
	return new jive_test_value_type;
}

static jive_input *
jive_test_value_type_create_input_(const jive_type * self_, jive_node * node,
	size_t index, jive_output * origin)
{
	return new jive_test_value_input(node, index, origin);
}

static jive_output *
jive_test_value_type_create_output_(const jive_type * self_, jive_node * node, size_t index)
{
	return new jive_test_value_output(node, index);
}

static jive_gate *
jive_test_value_type_create_gate_(const jive_type * self_, jive_graph * graph, const char * name)
{
	return new jive_test_value_gate(graph, name);
}

jive_test_value_input::~jive_test_value_input() noexcept {}

jive_test_value_input::jive_test_value_input(jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

jive_test_value_output::~jive_test_value_output() noexcept {}

jive_test_value_output::jive_test_value_output(jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

jive_test_value_gate::~jive_test_value_gate() noexcept {}

jive_test_value_gate::jive_test_value_gate(jive_graph * graph, const char name[])
	: jive_value_gate(graph, name)
{}

/* test state type */

static jive_input *
jive_test_state_type_create_input_(const jive_type * self, jive_node * node, size_t index,
	jive_output * origin);
static jive_output *
jive_test_state_type_create_output_(const jive_type * self, jive_node * node, size_t index);
static jive_gate *
jive_test_state_type_create_gate_(const jive_type * self, jive_graph * graph, const char name[]);
static jive_type *
jive_test_state_type_copy_(const jive_type * self);

const jive_type_class JIVE_TEST_STATE_TYPE = {
	parent : &JIVE_STATE_TYPE,
	name : "test_state",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_test_state_type_create_input_, /* override */
	create_output : jive_test_state_type_create_output_, /* override */
	create_gate : jive_test_state_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_test_state_type_copy_, /* override */
};

jive_test_state_type::~jive_test_state_type() noexcept {}

jive_test_state_type::jive_test_state_type() noexcept
	: jive_state_type(&JIVE_TEST_STATE_TYPE)
{}

void
jive_test_state_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "test_state");
}

bool
jive_test_state_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_test_state_type*>(&other) != nullptr;
}

std::unique_ptr<jive_type>
jive_test_state_type::copy() const
{
	return std::unique_ptr<jive_test_state_type>(new jive_test_state_type());
}

jive_input *
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

static jive_type *
jive_test_state_type_copy_(const jive_type * self_)
{
	return new jive_test_state_type;
}

static jive_input *
jive_test_state_type_create_input_(const jive_type * self_, jive_node * node,
	size_t index, jive_output * origin)
{
	return new jive_test_state_input(node, index, origin);
}

static jive_output *
jive_test_state_type_create_output_(const jive_type * self_, jive_node * node, size_t index)
{
	return new jive_test_state_output(node, index);
}

static jive_gate *
jive_test_state_type_create_gate_(const jive_type * self_, jive_graph * graph, const char * name)
{
	return new jive_test_state_gate(graph, name);
}

jive_test_state_input::~jive_test_state_input() noexcept {}

jive_test_state_input::jive_test_state_input(jive_node * node, size_t index,
	jive_output * origin)
	: jive_state_input(node, index, origin)
{}

jive_test_state_output::~jive_test_state_output() noexcept {}

jive_test_state_output::jive_test_state_output(jive_node * node, size_t index)
	: jive_state_output(node, index)
{}

jive_test_state_gate::~jive_test_state_gate() noexcept {}

jive_test_state_gate::jive_test_state_gate(jive_graph * graph, const char name[])
	: jive_state_gate(graph, name)
{}
