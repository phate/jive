/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/union/unntype.h>
#include <jive/util/buffer.h>
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

jive_union_type::~jive_union_type() noexcept {}

jive_union_type::jive_union_type(const jive_union_declaration * decl) noexcept
	: jive_value_type(&JIVE_UNION_TYPE)
	, decl_(decl)
{}

void
jive_union_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "unn");
}

bool
jive_union_type::operator==(const jive_type & _other) const noexcept
{
	const jive_union_type * other = dynamic_cast<const jive_union_type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

std::unique_ptr<jive_type>
jive_union_type::copy() const
{
	return std::unique_ptr<jive_type>(new jive_union_type(this->declaration()));
}

/* record_type inheritable members */

jive_type *
jive_union_type_copy_(const jive_type * self_)
{
	const jive_union_type * self = (const jive_union_type *) self_;

	jive_union_type * type = new jive_union_type(self->declaration());

	return type;
}

jive_input *
jive_union_type_create_input_(const jive_type * self_, struct jive_node * node,
	size_t index, jive_output * initial_operand)
{
	const jive_union_type * self = (const jive_union_type *) self_;
	jive_union_input * input = new jive_union_input(self->declaration(), node, index, initial_operand);
	return input;
}

jive_output *
jive_union_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_union_type * self = (const jive_union_type *) self_;
	return new jive_union_output(self->declaration(), node, index);
}

bool
jive_union_type_equals_(const jive_type * self_, const jive_type * other_)
{
	const jive_union_type * self = (const jive_union_type *) self_;
	const jive_union_type * other = (const jive_union_type *) other_;

	return (self->declaration() == other->declaration());
}

jive_gate *
jive_union_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	const jive_union_type * self = (const jive_union_type *) self_;
	return new jive_union_gate(self->declaration(), graph, name);
}

/* union_input inheritable members */

jive_union_input::jive_union_input(const jive_union_declaration * decl, struct jive_node * node,
	size_t index, jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(decl)
{}

jive_union_input::~jive_union_input() noexcept
{}

/* union_output inheritable members */

jive_union_output::jive_union_output(const jive_union_declaration * decl, jive_node * node,
	size_t index)
	: jive_value_output(node, index)
	, type_(decl)
{}

jive_union_output::~jive_union_output() noexcept
{}

/* union_gate inheritable members */

jive_union_gate::jive_union_gate(const jive_union_declaration * decl, jive_graph * graph,
	const char name[])
	: jive_value_gate(graph, name)
	, type_(decl)
{}

jive_union_gate::~jive_union_gate() noexcept {}
