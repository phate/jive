/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/symbolic-constant.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

static void
jive_bitsymbolicconstant_node_fini_(jive_node * self);

static void
jive_bitsymbolicconstant_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static const jive_node_attrs *
jive_bitsymbolicconstant_node_get_attrs_(const jive_node * self);

static bool
jive_bitsymbolicconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_bitsymbolicconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive::output * const operands[]);

const jive_node_class JIVE_BITSYMBOLICCONSTANT_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "BITSYMBOLICCONSTANT",
	fini : jive_bitsymbolicconstant_node_fini_, /* override */
	get_default_normal_form : jive_nullary_operation_get_default_normal_form_, /* inherit */
	get_label : jive_bitsymbolicconstant_node_get_label_, /* override */
	match_attrs : jive_bitsymbolicconstant_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_bitsymbolicconstant_node_create_, /* override */
};

static void
jive_bitsymbolicconstant_node_fini_(jive_node * self_)
{
	jive_bitsymbolicconstant_node * self = (jive_bitsymbolicconstant_node *) self_;
	jive_node_fini_(self);
}

static void
jive_bitsymbolicconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_bitsymbolicconstant_node * self = (const jive_bitsymbolicconstant_node *) self_;
	jive_buffer_putstr(buffer, self->operation().name.c_str());
}

static const jive_node_attrs *
jive_bitsymbolicconstant_node_get_attrs_(const jive_node * self_)
{
	const jive_bitsymbolicconstant_node * self = (const jive_bitsymbolicconstant_node *) self_;
	return &self->operation();
}

static bool
jive_bitsymbolicconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::bitstring::symbolicconstant_operation * first =
		&((const jive_bitsymbolicconstant_node *)self)->operation();
	const jive::bitstring::symbolicconstant_operation * second =
		(const jive::bitstring::symbolicconstant_operation *) attrs;
	if (first->nbits != second->nbits) return false;
	return first->name == second->name;
}

static jive_node *
jive_bitsymbolicconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive::output * const operands[])
{
	const jive::bitstring::symbolicconstant_operation * attrs =
		(const jive::bitstring::symbolicconstant_operation *) attrs_;
	jive_bitsymbolicconstant_node * node = new jive_bitsymbolicconstant_node(*attrs);
	node->class_ = &JIVE_BITSYMBOLICCONSTANT_NODE;
	jive::bits::type type(attrs->nbits);
	const jive::base::type * typeptr = &type;
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &typeptr);
	return node;
}

jive_node *
jive_bitsymbolicconstant_create(jive_graph * graph, size_t nbits, const char * name)
{
	jive::bitstring::symbolicconstant_operation attrs;
	attrs.nbits = nbits;
	attrs.name = name;

	return jive_nullary_operation_create_normalized(&JIVE_BITSYMBOLICCONSTANT_NODE, graph,
		&attrs)->node();
}

jive::output *
jive_bitsymbolicconstant(jive_graph * graph, size_t nbits, const char * name)
{
	jive::bitstring::symbolicconstant_operation attrs;
	attrs.nbits = nbits;
	attrs.name = name;

	return jive_nullary_operation_create_normalized(&JIVE_BITSYMBOLICCONSTANT_NODE, graph,
		&attrs);
}
