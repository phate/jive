/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testtypes.h"

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

static void
jive_test_value_input_init_(jive_test_value_input * self, const jive_test_value_type * type,
	jive_node * node, size_t index, jive_output * origin);
static void
jive_test_value_input_fini_(jive_input * self);
static const jive_type *
jive_test_value_input_get_type_(const jive_input * self);

static void
jive_test_value_output_init_(jive_test_value_output * self, const jive_test_value_type * type,
	jive_node * node, size_t index);
static void
jive_test_value_output_fini_(jive_output * self);
static const jive_type *
jive_test_value_output_get_type_(const jive_output * self);

static void
jive_test_value_gate_init_(jive_test_value_gate * self, const jive_test_value_type * type,
	jive_graph * graph, const char name[]);
static void
jive_test_value_gate_fini_(jive_gate * self);
static const jive_type *
jive_test_value_gate_get_type_(const jive_gate * self);

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

const jive_input_class JIVE_TEST_VALUE_INPUT = {
	parent : &JIVE_VALUE_INPUT,
	fini : jive_test_value_input_fini_, /* override */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_test_value_input_get_type_ /* override */
};

const jive_output_class JIVE_TEST_VALUE_OUTPUT = {
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_test_value_output_fini_, /* override */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_test_value_output_get_type_ /* override */
};

const jive_gate_class JIVE_TEST_VALUE_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_test_value_gate_fini_, /* override */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_test_value_gate_get_type_ /* override */
};

static void
jive_test_value_type_init(jive_test_value_type * self)
{
	self->class_ = &JIVE_TEST_VALUE_TYPE;
}

static jive_type *
jive_test_value_type_copy_(const jive_type * self_)
{
	jive_test_value_type * type = new jive_test_value_type;
	jive_test_value_type_init(type);
	return type;
}

static jive_input *
jive_test_value_type_create_input_(const jive_type * self_, jive_node * node,
	size_t index, jive_output * origin)
{
	const jive_test_value_type * self = (const jive_test_value_type *) self_;
	jive_test_value_input * input = new jive_test_value_input;
	input->class_ = &JIVE_TEST_VALUE_INPUT;
	jive_test_value_input_init_(input, self, node, index, origin);
	return input;
}

static jive_output *
jive_test_value_type_create_output_(const jive_type * self_, jive_node * node, size_t index)
{
	const jive_test_value_type * self = (const jive_test_value_type *) self_;
	jive_test_value_output * output = new jive_test_value_output;
	output->class_ = &JIVE_TEST_VALUE_OUTPUT;
	jive_test_value_output_init_(output, self, node, index);
	return output;
}

static jive_gate *
jive_test_value_type_create_gate_(const jive_type * self_, jive_graph * graph, const char * name)
{
	const jive_test_value_type * self = (const jive_test_value_type *) self_;
	jive_test_value_gate * gate = new jive_test_value_gate;
	gate->class_ = &JIVE_TEST_VALUE_GATE;
	jive_test_value_gate_init_(gate, self, graph, name);
	return gate;
}

static void
jive_test_value_input_init_(jive_test_value_input * self, const jive_test_value_type * type,
	jive_node * node, size_t index, jive_output * origin)
{
	jive_value_input_init_(self, node, index, origin);
	jive_test_value_type_init(&self->type);
}

static void
jive_test_value_input_fini_(jive_input * self_)
{
	jive_test_value_input * self = (jive_test_value_input *) self_;
	jive_type_fini(&self->type);
	jive_input_fini_(self);
}

static const jive_type *
jive_test_value_input_get_type_(const jive_input * self_)
{
	const jive_test_value_input * self = (const jive_test_value_input *) self_;
	return &self->type;
}

static void
jive_test_value_output_init_(jive_test_value_output * self, const jive_test_value_type * type,
	jive_node * node, size_t index)
{
	jive_value_output_init_(self, node, index);
	jive_test_value_type_init(&self->type);
}

static void
jive_test_value_output_fini_(jive_output * self_)
{
	jive_test_value_output * self = (jive_test_value_output *) self_;
	jive_type_fini(&self->type);
	jive_output_fini_(self);
}

static const jive_type *
jive_test_value_output_get_type_(const jive_output * self_)
{
	const jive_test_value_output * self = (const jive_test_value_output *) self_;
	return &self->type;
}

static void
jive_test_value_gate_init_(jive_test_value_gate * self, const jive_test_value_type * type,
	jive_graph * graph, const char name[])
{
	jive_value_gate_init_(self, graph, name);
	jive_test_value_type_init(&self->type);
}

static void
jive_test_value_gate_fini_(jive_gate * self_)
{
	jive_test_value_gate * self = (jive_test_value_gate *) self_;
	jive_type_fini(&self->type);
	jive_gate_fini_(self);
}

static const jive_type *
jive_test_value_gate_get_type_(const jive_gate * self_)
{
	const jive_test_value_gate * self = (const jive_test_value_gate *) self_;
	return &self->type;
}

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

static void
jive_test_state_input_init_(jive_test_state_input * self, const jive_test_state_type * type,
	jive_node * node, size_t index, jive_output * origin);
static void
jive_test_state_input_fini_(jive_input * self);
static const jive_type *
jive_test_state_input_get_type_(const jive_input * self);

static void
jive_test_state_output_init_(jive_test_state_output * self, const jive_test_state_type * type,
	jive_node * node, size_t index);
static void
jive_test_state_output_fini_(jive_output * self);
static const jive_type *
jive_test_state_output_get_type_(const jive_output * self);

static void
jive_test_state_gate_init_(jive_test_state_gate * self, const jive_test_state_type * type,
	jive_graph * graph, const char name[]);
static void
jive_test_state_gate_fini_(jive_gate * self);
static const jive_type *
jive_test_state_gate_get_type_(const jive_gate * self);

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

const jive_input_class JIVE_TEST_STATE_INPUT = {
	parent : &JIVE_STATE_INPUT,
	fini : jive_test_state_input_fini_, /* override */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_test_state_input_get_type_ /* override */
};

const jive_output_class JIVE_TEST_STATE_OUTPUT = {
	parent : &JIVE_STATE_OUTPUT,
	fini : jive_test_state_output_fini_, /* override */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_test_state_output_get_type_ /* override */
};

const jive_gate_class JIVE_TEST_STATE_GATE = {
	parent : &JIVE_STATE_GATE,
	fini : jive_test_state_gate_fini_, /* override */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_test_state_gate_get_type_ /* override */
};

static void
jive_test_state_type_init(jive_test_state_type * self)
{
	self->class_ = &JIVE_TEST_STATE_TYPE;
}

static jive_type *
jive_test_state_type_copy_(const jive_type * self_)
{
	jive_test_state_type * type = new jive_test_state_type;
	jive_test_state_type_init(type);
	return type;
}

static jive_input *
jive_test_state_type_create_input_(const jive_type * self_, jive_node * node,
	size_t index, jive_output * origin)
{
	const jive_test_state_type * self = (const jive_test_state_type *) self_;
	jive_test_state_input * input = new jive_test_state_input;
	input->class_ = &JIVE_TEST_STATE_INPUT;
	jive_test_state_input_init_(input, self, node, index, origin);
	return input;
}

static jive_output *
jive_test_state_type_create_output_(const jive_type * self_, jive_node * node, size_t index)
{
	const jive_test_state_type * self = (const jive_test_state_type *) self_;
	jive_test_state_output * output = new jive_test_state_output;
	output->class_ = &JIVE_TEST_STATE_OUTPUT;
	jive_test_state_output_init_(output, self, node, index);
	return output;
}

static jive_gate *
jive_test_state_type_create_gate_(const jive_type * self_, jive_graph * graph, const char * name)
{
	const jive_test_state_type * self = (const jive_test_state_type *) self_;
	jive_test_state_gate * gate = new jive_test_state_gate;
	gate->class_ = &JIVE_TEST_STATE_GATE;
	jive_test_state_gate_init_(gate, self, graph, name);
	return gate;
}

static void
jive_test_state_input_init_(jive_test_state_input * self, const jive_test_state_type * type,
	jive_node * node, size_t index, jive_output * origin)
{
	jive_state_input_init_(self, node, index, origin);
	jive_test_state_type_init(&self->type);
}

static void
jive_test_state_input_fini_(jive_input * self_)
{
	jive_test_state_input * self = (jive_test_state_input *) self_;
	jive_type_fini(&self->type);
	jive_input_fini_(self);
}

static const jive_type *
jive_test_state_input_get_type_(const jive_input * self_)
{
	const jive_test_state_input * self = (const jive_test_state_input *) self_;
	return &self->type;
}

static void
jive_test_state_output_init_(jive_test_state_output * self, const jive_test_state_type * type,
	jive_node * node, size_t index)
{
	jive_state_output_init_(self, node, index);
	jive_test_state_type_init(&self->type);
}

static void
jive_test_state_output_fini_(jive_output * self_)
{
	jive_test_state_output * self = (jive_test_state_output *) self_;
	jive_type_fini(&self->type);
	jive_output_fini_(self);
}

static const jive_type *
jive_test_state_output_get_type_(const jive_output * self_)
{
	const jive_test_state_output * self = (const jive_test_state_output *) self_;
	return &self->type;
}

static void
jive_test_state_gate_init_(jive_test_state_gate * self, const jive_test_state_type * type,
	jive_graph * graph, const char name[])
{
	jive_state_gate_init_(self, graph, name);
	jive_test_state_type_init(&self->type);
}

static void
jive_test_state_gate_fini_(jive_gate * self_)
{
	jive_test_state_gate * self = (jive_test_state_gate *) self_;
	jive_type_fini(&self->type);
	jive_gate_fini_(self);
}

static const jive_type *
jive_test_state_gate_get_type_(const jive_gate * self_)
{
	const jive_test_state_gate * self = (const jive_test_state_gate *) self_;
	return &self->type;
}
