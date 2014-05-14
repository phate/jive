/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <stdio.h>
#include <string.h>

static void
jive_bitstring_type_get_label_(const jive_type * self, struct jive_buffer * buffer);

static jive_input *
jive_bitstring_type_create_input_(const jive_type * self, struct jive_node * node, size_t index,
	jive_output * initial_operand);

static jive_output *
jive_bitstring_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

static jive_gate *
jive_bitstring_type_create_gate_(const jive_type * self, struct jive_graph * graph,
	const char * name);

static jive_type *
jive_bitstring_type_copy_(const jive_type * self);

static bool
jive_bitstring_type_equals_(const jive_type * self, const jive_type * other);

const jive_type_class JIVE_BITSTRING_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "bit",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_bitstring_type_get_label_, /* override */
	create_input : jive_bitstring_type_create_input_, /* override */
	create_output : jive_bitstring_type_create_output_, /* override */
	create_gate : jive_bitstring_type_create_gate_, /* override */
	equals : jive_bitstring_type_equals_, /* override */
	copy : jive_bitstring_type_copy_, /* override */
};

/* bitstring_type inheritable members */

jive_bitstring_type::~jive_bitstring_type() noexcept {}

jive_bitstring_type::jive_bitstring_type(size_t nbits) noexcept
	: jive_value_type(&JIVE_BITSTRING_TYPE)
	, nbits_(nbits)
{}


void
jive_bitstring_type::label(jive_buffer & buffer) const
{
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "bits%zd", nbits());
	jive_buffer_putstr(&buffer, tmp);
}

bool
jive_bitstring_type::operator==(const jive_type & _other) const noexcept
{
	const jive_bitstring_type * other = dynamic_cast<const jive_bitstring_type*>(&_other);
	return other != nullptr && this->nbits() == other->nbits();
}

std::unique_ptr<jive_type>
jive_bitstring_type::copy() const
{
	return std::unique_ptr<jive_type>(new jive_bitstring_type(this->nbits()));
}

jive_input *
jive_bitstring_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_bitstring_input(nbits_, node, index, origin);
}

jive_output *
jive_bitstring_type::create_output(jive_node * node, size_t index) const
{
	return new jive_bitstring_output(nbits(), node, index);
}

static void
jive_bitstring_type_get_label_(const jive_type * self_, struct jive_buffer * buffer)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "bits%zd", self->nbits());
	jive_buffer_putstr(buffer, tmp);
}

static jive_input *
jive_bitstring_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_input * input = new jive_bitstring_input(self->nbits(), node, index,
		initial_operand);
	return input;
}

static jive_output *
jive_bitstring_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	return new jive_bitstring_output(self->nbits(), node, index);
}

static jive_gate *
jive_bitstring_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	return new jive_bitstring_gate(self->nbits(), graph, name);
}

static jive_type *
jive_bitstring_type_copy_(const jive_type * self_)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	
	jive_bitstring_type * type = new jive_bitstring_type(self->nbits());
	
	return type;
}

static bool
jive_bitstring_type_equals_(const jive_type * self_, const jive_type * other_)
{
	if (self_->class_ != other_->class_) return false;
	const jive_bitstring_type * self = (const jive_bitstring_type *)self_;
	const jive_bitstring_type * other = (const jive_bitstring_type *)other_;
	
	return self->nbits() == other->nbits();
}

/* bitstring_input inheritable members */

jive_bitstring_input::jive_bitstring_input(size_t nbits, struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(nbits)
{}

jive_bitstring_input::~jive_bitstring_input() noexcept {}

/* bitstring_output inheritable members */

jive_bitstring_output::jive_bitstring_output(size_t nbits, struct jive_node * node,
	size_t index)
	: jive_value_output(node, index)
	, type_(nbits)
{}

jive_bitstring_output::~jive_bitstring_output() noexcept {}

/* bitstring_gate inheritable members */

jive_bitstring_gate::jive_bitstring_gate(size_t nbits, jive_graph * graph,
	const char name[])
	: jive_value_gate(graph, name)
	, type_(nbits)
{}

jive_bitstring_gate::~jive_bitstring_gate() noexcept {}
