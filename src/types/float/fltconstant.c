/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
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
jive_fltconstant_node_init_(jive_fltconstant_node * self, jive_region * region, uint32_t value);

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
	.parent = &JIVE_NULLARY_OPERATION,
	.name = "FLTCONSTANT",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_nullary_operation_get_default_normal_form_, /* inherit */
	.get_label = jive_fltconstant_node_get_label_, /* override */
	.get_attrs = jive_fltconstant_node_get_attrs_, /* override */
	.match_attrs = jive_fltconstant_node_match_attrs_, /* override */
	.check_operands = jive_node_check_operands_, /* inherit */
	.create = jive_fltconstant_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_fltconstant_node_init_(jive_fltconstant_node * self, jive_region * region, uint32_t value)
{
	JIVE_DECLARE_FLOAT_TYPE(flttype);
	jive_node_init_(&self->base, region,
		0, NULL, NULL,
		1, &flttype);
	
	self->attrs.value = value;
}


static void
jive_fltconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_fltconstant_node * self = (const jive_fltconstant_node *) self_;

	union u {
		uint32_t i;
		float f;
	};

	union u c;
	c.i = self->attrs.value;	

	char tmp[80];
	snprintf(tmp, sizeof(tmp), "%f", c.f);
	jive_buffer_putstr(buffer, tmp);
}

static const jive_node_attrs *
jive_fltconstant_node_get_attrs_(const jive_node * self_)
{
	const jive_fltconstant_node * self = (const jive_fltconstant_node *) self_;
	return &self->attrs.base;
}

static bool
jive_fltconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_fltconstant_node_attrs * first = &((const jive_fltconstant_node *) self)->attrs;
	const jive_fltconstant_node_attrs * second = (const jive_fltconstant_node_attrs *) attrs;

	if(first->value != second->value)
		return false;
	
	return true;
}

static jive_node *
jive_fltconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_fltconstant_node_attrs * attrs = (const jive_fltconstant_node_attrs *) attrs_;

	jive_fltconstant_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_FLTCONSTANT_NODE;
	jive_fltconstant_node_init_(node, region, attrs->value);

	return &node->base;
}

jive_output *
jive_fltconstant(struct jive_graph * graph, uint32_t value)
{
	jive_fltconstant_node_attrs attrs;
	attrs.value = value;

	return jive_nullary_operation_create_normalized(&JIVE_FLTCONSTANT_NODE, graph, &attrs.base);
}
