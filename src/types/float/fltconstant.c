/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/fltconstant.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/float/fltoperation-classes-private.h>
#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators/nullary.h>

#include <stdio.h>
#include <string.h>

static void
jive_fltconstant_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static const jive_node_attrs *
jive_fltconstant_node_get_attrs_(const jive_node * self);

static bool
jive_fltconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_fltconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_FLTCONSTANT_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "FLTCONSTANT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_nullary_operation_get_default_normal_form_, /* inherit */
	get_label : jive_fltconstant_node_get_label_, /* override */
	get_attrs : nullptr,
	match_attrs : jive_fltconstant_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_fltconstant_node_create_, /* override */
};

static void
jive_fltconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_fltconstant_node * self = (const jive_fltconstant_node *) self_;

	union u {
		uint32_t i;
		float f;
	};

	union u c;
	c.i = self->operation().value();

	char tmp[80];
	snprintf(tmp, sizeof(tmp), "%f", c.f);
	jive_buffer_putstr(buffer, tmp);
}

static bool
jive_fltconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::flt::constant_operation * first = &((const jive_fltconstant_node *) self)->operation();
	const jive::flt::constant_operation * second = (const jive::flt::constant_operation *) attrs;

	return first->value() == second->value();
}

static jive_node *
jive_fltconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive::flt::constant_operation * attrs = (const jive::flt::constant_operation *) attrs_;

	jive_fltconstant_node * node = new jive_fltconstant_node(*attrs);
	node->class_ = &JIVE_FLTCONSTANT_NODE;
	jive_float_type flttype;
	const jive_type * tmparray0[] = {&flttype};
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, tmparray0);

	return node;
}

jive_output *
jive_fltconstant(struct jive_graph * graph, uint32_t value)
{
	jive::flt::constant_operation attrs(value);

	return jive_nullary_operation_create_normalized(&JIVE_FLTCONSTANT_NODE, graph, &attrs);
}
