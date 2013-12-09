/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

static void
jive_bitconstant_node_init_(
	jive_bitconstant_node * self,
	jive_region * region,
	size_t nbits, const char bits[]);

static void
jive_bitconstant_node_fini_(jive_node * self);

static void
jive_bitconstant_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static const jive_node_attrs *
jive_bitconstant_node_get_attrs_(const jive_node * self);

static bool
jive_bitconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_bitconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_BITCONSTANT_NODE = {
	.parent = &JIVE_NULLARY_OPERATION,
	.name = "BITCONSTANT",
	.fini = jive_bitconstant_node_fini_, /* override */
	.get_default_normal_form = jive_nullary_operation_get_default_normal_form_, /* inherit */
	.get_label = jive_bitconstant_node_get_label_, /* override */
	.get_attrs = jive_bitconstant_node_get_attrs_, /* override */
	.match_attrs = jive_bitconstant_node_match_attrs_, /* override */
	.check_operands = jive_node_check_operands_, /* inherit */
	.create = jive_bitconstant_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_bitconstant_node_init_(
	jive_bitconstant_node * self,
	jive_region * region,
	size_t nbits, const char bits[])
{
	if (nbits == 0)
		jive_context_fatal_error(region->graph->context,
			"bitconstant: number of bits must be greater than zero.");

	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	jive_node_init_(&self->base, region,
		0, NULL, NULL,
		1, &type);
	self->attrs.nbits = nbits;
	self->attrs.bits = jive_context_malloc(region->graph->context, nbits);
	size_t n;
	for(n=0; n<nbits; n++) self->attrs.bits[n] = bits[n];
}

static void
jive_bitconstant_node_fini_(jive_node * self_)
{
	jive_bitconstant_node * self = (jive_bitconstant_node *) self_;
	jive_context_free(self->base.graph->context, self->attrs.bits);
	jive_node_fini_(&self->base);
}

static void
jive_bitconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_bitconstant_node * self = (const jive_bitconstant_node *) self_;
	
	char tmp[self->attrs.nbits + 1];
	size_t n;
	for(n=0; n<self->attrs.nbits; n++)
		tmp[n] = self->attrs.bits[self->attrs.nbits - n - 1];
	tmp[n] = 0;
	jive_buffer_putstr(buffer, tmp);
}

static const jive_node_attrs *
jive_bitconstant_node_get_attrs_(const jive_node * self_)
{
	const jive_bitconstant_node * self = (const jive_bitconstant_node *) self_;
	return &self->attrs.base;
}

static bool
jive_bitconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_bitconstant_node_attrs * first = &((const jive_bitconstant_node *) self)->attrs;
	const jive_bitconstant_node_attrs * second = (const jive_bitconstant_node_attrs *) attrs;
	if (first->nbits != second->nbits) return false;
	size_t n;
	for(n=0; n<first->nbits; n++)
		if (first->bits[n] != second->bits[n]) return false;
	return true;
}

static jive_node *
jive_bitconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);

	const jive_bitconstant_node_attrs * attrs = (const jive_bitconstant_node_attrs *) attrs_;
	
	jive_bitconstant_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_BITCONSTANT_NODE;
	jive_bitconstant_node_init_(node, region, attrs->nbits, attrs->bits);
	
	return &node->base;
}

jive_output *
jive_bitconstant(jive_graph * graph, size_t nbits, const char bits[])
{
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = (char *) bits;

	return jive_nullary_operation_create_normalized(&JIVE_BITCONSTANT_NODE, graph, &attrs.base);
}

jive_output *
jive_bitconstant_unsigned(struct jive_graph * graph, size_t nbits, uint64_t value)
{
	char bits[nbits];
	jive_bitstring_init_unsigned(bits, nbits, value);
	
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = bits;

	return jive_nullary_operation_create_normalized(&JIVE_BITCONSTANT_NODE, graph, &attrs.base);
}

jive_output *
jive_bitconstant_signed(struct jive_graph * graph, size_t nbits, int64_t value)
{
	char bits[nbits];
	jive_bitstring_init_signed(bits, nbits, value);
	
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = bits;

	return jive_nullary_operation_create_normalized(&JIVE_BITCONSTANT_NODE, graph, &attrs.base);
}
