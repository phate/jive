/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/union/unntype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

/*union_type inheritable members*/

static jive_input *
jive_union_type_create_input_(const jive_type * self, struct jive_node * node,
	size_t index, jive_output * initial_operand);
static jive_output *
jive_union_type_create_output_(const jive_type * self, struct jive_node * node,
	size_t index);
static jive_gate *
jive_union_type_create_gate_(const jive_type * self, struct jive_graph * graph,
	const char name[]);
static bool
jive_union_type_equals_(const jive_type * self, const jive_type * other);
static jive_type *
jive_union_type_copy_(const jive_type * self);

static void
jive_union_input_init_(jive_union_input * self, const jive_union_type * type,
	struct jive_node * node, size_t index, jive_output * origin);
static void
jive_union_input_fini_(jive_input * self);
static const jive_type *
jive_union_input_get_type_(const jive_input * self);

static void
jive_union_output_init_(jive_union_output * self, const jive_union_type * type,
	struct jive_node * node, size_t index);
static void
jive_union_output_fini_(jive_output * self);
static const jive_type *
jive_union_output_get_type_(const jive_output * self);

static void
jive_union_gate_init_(jive_union_gate * self, const jive_union_type * type,
	struct jive_graph * graph, const char name[]);
static void
jive_union_gate_fini_(jive_gate * self);
static const jive_type *
jive_union_gate_get_type_(const jive_gate * self);

const jive_type_class JIVE_UNION_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "union",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_union_type_create_input_, /* override */
	create_output : jive_union_type_create_output_, /* overrride */
	create_gate : jive_union_type_create_gate_, /* override */
	equals : jive_union_type_equals_, /* override */
	copy : jive_union_type_copy_, /* override */
};

const jive_input_class JIVE_UNION_INPUT = {
	parent : &JIVE_VALUE_INPUT,
	fini : jive_union_input_fini_, /* override */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_union_input_get_type_, /* override */
};

const jive_output_class JIVE_UNION_OUTPUT = {
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_union_output_fini_, /* override */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_union_output_get_type_, /* override */
};

const jive_gate_class JIVE_UNION_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_union_gate_fini_, /* override */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_union_gate_get_type_, /* override */
};

void
jive_union_type_init(jive_union_type * self, const jive_union_declaration * decl)
{
	self->class_ = &JIVE_UNION_TYPE;
	self->decl = decl;
}

/* record_type inheritable members */

jive_type *
jive_union_type_copy_(const jive_type * self_)
{
	const jive_union_type * self = (const jive_union_type *) self_;

	jive_union_type * type = new jive_union_type;

	jive_union_type_init(type, self->decl);

	return type;
}

jive_input *
jive_union_type_create_input_(const jive_type * self_, struct jive_node * node,
	size_t index, jive_output * initial_operand)
{
	const jive_union_type * self = (const jive_union_type *) self_;
	jive_union_input * input = jive_context_malloc(node->graph->context, sizeof(*input));

	input->class_ = &JIVE_UNION_INPUT;
	jive_union_input_init_(input, self, node, index, initial_operand);

	return input;
}

jive_output *
jive_union_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_union_type * self = (const jive_union_type *) self_;
	jive_union_output * output = jive_context_malloc(node->graph->context, sizeof(*output));

	output->class_ = &JIVE_UNION_OUTPUT;
	jive_union_output_init_(output, self, node, index);

	return output;
}

bool
jive_union_type_equals_(const jive_type * self_, const jive_type * other_)
{
	const jive_union_type * self = (const jive_union_type *) self_;
	const jive_union_type * other = (const jive_union_type *) other_;

	return (self->decl == other->decl);
}

jive_gate *
jive_union_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	const jive_union_type * self = (const jive_union_type *) self_;

	jive_union_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));

	gate->class_ = &JIVE_UNION_GATE;
	jive_union_gate_init_(gate, self, graph, name);

	return gate;
}

/* union_input inheritable members */

void
jive_union_input_init_(jive_union_input * self, const jive_union_type * type,
	struct jive_node * node, size_t index, jive_output * origin)
{
	jive_value_input_init_(self, node, index, origin);
	jive_union_type_init(&self->type, type->decl);
}

void
jive_union_input_fini_(jive_input * self_)
{
	jive_union_input * self = (jive_union_input *) self_;

	self->type.class_->fini(&self->type);
	jive_input_fini_(self);
}

const jive_type *
jive_union_input_get_type_(const jive_input * self_)
{
	const jive_union_input * self = (const jive_union_input *) self_;
	return &self->type;
}

/* union_output inheritable members */

void
jive_union_output_init_(jive_union_output * self, const jive_union_type * type,
	struct jive_node * node, size_t index)
{
	jive_value_output_init_(self, node, index);
	jive_union_type_init(&self->type, type->decl);
}

void
jive_union_output_fini_(jive_output * self_)
{
	jive_union_output * self = (jive_union_output *) self_;

	self->type.class_->fini(&self->type);
	jive_output_fini_(self);
}

const jive_type *
jive_union_output_get_type_(const jive_output *self_)
{
	const jive_union_output * self = (const jive_union_output *) self_;

	return &self->type;
}

/* union_gate inheritable members */

void
jive_union_gate_init_(jive_union_gate * self, const jive_union_type * type,
	struct jive_graph * graph, const char name[])
{
	jive_value_gate_init_(self, graph, name);
	jive_union_type_init(&self->type, type->decl);
}

void
jive_union_gate_fini_(jive_gate * self_)
{
	jive_union_gate * self = (jive_union_gate *) self_;

	jive_type_fini(&self->type);
	jive_gate_fini_(self);
}

const jive_type *
jive_union_gate_get_type_(const jive_gate * self_)
{
	const jive_union_gate * self = (const jive_union_gate *) self_;

	return &self->type;
}
