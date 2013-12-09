/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/integral/itgconstant.h>
#include <jive/types/integral/itgtype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/node-private.h>

#include <string.h>

static void
jive_itgconstant_node_fini_(jive_node * self);

static void
jive_itgconstant_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static const jive_node_attrs *
jive_itgconstant_node_get_attrs_(const jive_node * self);

static bool
jive_itgconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_itgconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_nullary_operation_class JIVE_ITGCONSTANT_NODE = {
	.parent = &JIVE_NULLARY_OPERATION,
	.name = "ITGCONSTANT",
	.fini = jive_itgconstant_node_fini_, /* override */
	.get_default_normal_form = jive_nullary_operation_get_default_normal_form_, /* inherit */
	.get_label = jive_itgconstant_node_get_label_, /* override */
	.get_attrs = jive_itgconstant_node_get_attrs_, /* override */
	.match_attrs = jive_itgconstant_node_match_attrs_, /* override */
	.check_operands = jive_node_check_operands_, /* inherit */
	.create = jive_itgconstant_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_itgconstant_node_init_(jive_itgconstant_node * self, struct jive_region * region,
	size_t nbits, const char bits[])
{
	JIVE_DEBUG_ASSERT(nbits != 0);
	JIVE_DECLARE_INTEGRAL_TYPE(itgtype);
	jive_node_init_(&self->base, region,
		0, NULL, NULL,
		1, &itgtype);
	self->attrs.nbits = nbits;
	self->attrs.bits = jive_context_malloc(region->graph->context, nbits);
	size_t n;
	for (n = 0; n < nbits; n++)
		self->attrs.bits[n] = bits[n];
}

static void
jive_itgconstant_node_fini_(jive_node * self_)
{
	jive_itgconstant_node * self = (jive_itgconstant_node *) self_;
	jive_context_free(self->base.graph->context, self->attrs.bits);
	jive_node_fini_(&self->base);
}

static void
jive_itgconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_itgconstant_node * self = (const jive_itgconstant_node *) self_;

	size_t n;
	char tmp[self->attrs.nbits + 1];
	for (n = 0; n < self->attrs.nbits; n++)
		tmp[n] = self->attrs.bits[self->attrs.nbits - n - 1];
	tmp[n] = 0;
	jive_buffer_putstr(buffer, tmp);
}

static const jive_node_attrs *
jive_itgconstant_node_get_attrs_(const jive_node * self_)
{
	const jive_itgconstant_node * self = (const jive_itgconstant_node *) self_;
	return &self->attrs.base;
}

static bool
jive_itgconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_itgconstant_node_attrs * first = &((const jive_itgconstant_node *) self)->attrs;
	const jive_itgconstant_node_attrs * second = (const jive_itgconstant_node_attrs *) attrs;

	if (first->nbits != second->nbits)
		return false;

	size_t n;
	for (n = 0; n < first->nbits; n++)
		if (first->bits[n] != second->bits[n])
			return false;

	return true;
}

static jive_node *
jive_itgconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);
	const jive_itgconstant_node_attrs * attrs = (const jive_itgconstant_node_attrs *) attrs_;

	jive_itgconstant_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_ITGCONSTANT_NODE;
	jive_itgconstant_node_init_(node, region, attrs->nbits, attrs->bits);

	return &node->base;
}

struct jive_output *
jive_itgconstant(struct jive_graph * graph, size_t nbits, const char bits[])
{
	jive_itgconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = (char *) bits;

	return jive_nullary_operation_create_normalized(&JIVE_ITGCONSTANT_NODE, graph, &attrs.base);
}
