/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdtype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

/* record_type inheritable members */

static jive_input *
jive_record_type_create_input_(const jive_type * self, struct jive_node * node,
	size_t index, jive_output * initial_operand);
static jive_output *
jive_record_type_create_output_(const jive_type * self, struct jive_node * node,
	size_t index);
static jive_gate *
jive_record_type_create_gate_(const jive_type * self, struct jive_graph * graph,
	const char name[]);
static bool
jive_record_type_equals_(const jive_type * self, const jive_type * other);
static jive_type *
jive_record_type_copy_(const jive_type * self);

const jive_type_class JIVE_RECORD_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "rcd",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_record_type_create_input_, /* override */
	create_output : jive_record_type_create_output_, /* override */
	create_gate : jive_record_type_create_gate_, /* override */
	equals : jive_record_type_equals_, /* override */
	copy : jive_record_type_copy_, /* override */
} ;

jive_record_type::~jive_record_type() noexcept {}

jive_record_type::jive_record_type(const jive_record_declaration * decl) noexcept
	: jive_value_type(&JIVE_RECORD_TYPE)
	, decl_(decl)
{}

void
jive_record_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "rcd");
}

bool
jive_record_type::operator==(const jive_type & _other) const noexcept
{
	const jive_record_type * other = dynamic_cast<const jive_record_type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

std::unique_ptr<jive_type>
jive_record_type::copy() const
{
	return std::unique_ptr<jive_type>(new jive_record_type(this->declaration()));
}

jive_input *
jive_record_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_record_input(this->declaration(), node, index, origin);
}

jive_output *
jive_record_type::create_output(jive_node * node, size_t index) const
{
	return new jive_record_output(this->declaration(), node, index);
}

/* record_type inheritable members */

jive_type *
jive_record_type_copy_(const jive_type * self_)
{
	const jive_record_type * self = (const jive_record_type *) self_;

	return new jive_record_type(self->declaration());
}

jive_input *
jive_record_type_create_input_(const jive_type * self_, struct jive_node * node,
	size_t index, jive_output * initial_operand)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	return new jive_record_input(self->declaration(), node, index, initial_operand);
}

jive_output *
jive_record_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	return new jive_record_output(self->declaration(), node, index);
}

bool
jive_record_type_equals_(const jive_type * self_, const jive_type * other_)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	const jive_record_type * other = (const jive_record_type *) other_;

	return (self->declaration() == other->declaration()) ;
}

jive_gate *
jive_record_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	return new jive_record_gate(self->declaration(), graph, name);
}

/* record_input inheritable members */

jive_record_input::jive_record_input(const jive_record_declaration * decl, struct jive_node * node,
	size_t index, jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(decl)
{}

jive_record_input::~jive_record_input() noexcept {}

/* record_output inheritable members */

jive_record_output::jive_record_output(const jive_record_declaration * decl, struct jive_node * node,
	size_t index)
	: jive_value_output(node, index)
	, type_(decl)
{}

jive_record_output::~jive_record_output() noexcept {}

/* record_gate inheritable members */

jive_record_gate::jive_record_gate(const jive_record_declaration * decl, jive_graph * graph,
	const char name[])
	: jive_value_gate(graph, name)
	, type_(decl)
{}

jive_record_gate::~jive_record_gate() noexcept {}
