/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdtype.h>
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

static void
jive_record_output_init_(jive_record_output * self, const jive_record_type * type,
	struct jive_node * node, size_t index);
static void
jive_record_output_fini_(jive_output * self);
static const jive_type *
jive_record_output_get_type_(const jive_output * self);

static void
jive_record_gate_init_(jive_record_gate * self, const jive_record_type * type,
	struct jive_graph * graph, const char name[]);
static void
jive_record_gate_fini_(jive_gate * self);
static const jive_type *
jive_record_gate_get_type_(const jive_gate * self);

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

const jive_output_class JIVE_RECORD_OUTPUT = { 
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_record_output_fini_, /* override */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_record_output_get_type_, /* override */
} ;

const jive_gate_class JIVE_RECORD_GATE = { 
	parent : &JIVE_VALUE_GATE,
	fini : jive_record_gate_fini_, /* override */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_record_gate_get_type_, /* override */
} ;

jive_record_type::~jive_record_type() noexcept {}

jive_record_type::jive_record_type(const jive_record_declaration * decl) noexcept
	: jive_value_type(&JIVE_RECORD_TYPE)
	, decl_(decl)
{}

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
	: jive_value_output(&JIVE_RECORD_OUTPUT, node, index)
	, type_(decl)
{}

jive_record_output::~jive_record_output() noexcept {}

void
jive_record_output_fini_(jive_output * self_)
{
}

const jive_type *
jive_record_output_get_type_(const jive_output * self_)
{
	const jive_record_output * self = (const jive_record_output *) self_;

	return &self->type();
}

/* record_gate inheritable members */

jive_record_gate::jive_record_gate(const jive_record_declaration * decl, jive_graph * graph,
	const char name[])
	: jive_value_gate(&JIVE_RECORD_GATE, graph, name)
	, type_(decl)
{}

jive_record_gate::~jive_record_gate() noexcept {}

void
jive_record_gate_fini_(jive_gate * self_)
{
}

const jive_type *
jive_record_gate_get_type_(const jive_gate * self_)
{
	const jive_record_gate * self = (const jive_record_gate *) self_;

	return &self->type();
}
