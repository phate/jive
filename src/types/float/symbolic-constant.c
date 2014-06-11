/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/symbolic-constant.h>

#include <string.h>

#include <jive/types/float/flttype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

static void
jive_fltsymbolicconstant_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static bool
jive_fltsymbolicconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_fltsymbolicconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[]);

const jive_node_class JIVE_FLTSYMBOLICCONSTANT_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "FLTSYMBOLICCONSTANT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_nullary_operation_get_default_normal_form_, /* inherit */
	get_label : jive_fltsymbolicconstant_node_get_label_, /* override */
	match_attrs : jive_fltsymbolicconstant_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_fltsymbolicconstant_node_create_, /* override */
};

static void
jive_fltsymbolicconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_fltsymbolicconstant_node * self = (const jive_fltsymbolicconstant_node *) self_;
	jive_buffer_putstr(buffer, self->operation().name.c_str());
}

static bool
jive_fltsymbolicconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::flt::symbolicconstant_operation * first =
		&((const jive_fltsymbolicconstant_node *)self)->operation();
	const jive::flt::symbolicconstant_operation * second =
		(const jive::flt::symbolicconstant_operation *) attrs;
	return first->name == second->name;
}

static jive_node *
jive_fltsymbolicconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[])
{
	const jive::flt::symbolicconstant_operation * attrs =
		(const jive::flt::symbolicconstant_operation *) attrs_;
	jive_fltsymbolicconstant_node * node = new jive_fltsymbolicconstant_node(*attrs);
	node->class_ = &JIVE_FLTSYMBOLICCONSTANT_NODE;
	jive::flt::type type;
	const jive::base::type * typeptr = &type;
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &typeptr);
	return node;
}

jive::output *
jive_fltsymbolicconstant(jive_graph * graph, const char * name)
{
	jive::flt::symbolicconstant_operation attrs;
	attrs.name = name;

	return jive_nullary_operation_create_normalized(&JIVE_FLTSYMBOLICCONSTANT_NODE, graph,
		&attrs);
}
