/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/symbolic-constant.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
jive_bitsymbolicconstant_node_init_(
	jive_bitsymbolicconstant_node * self,
	jive_region * region,
	size_t nbits, const char bits[]);

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
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_BITSYMBOLICCONSTANT_NODE = {
	.parent = &JIVE_NULLARY_OPERATION,
	.fini = jive_bitsymbolicconstant_node_fini_, /* override */
	.get_default_normal_form = jive_nullary_operation_get_default_normal_form_, /* inherit */
	.get_label = jive_bitsymbolicconstant_node_get_label_, /* override */
	.get_attrs = jive_bitsymbolicconstant_node_get_attrs_, /* override */
	.match_attrs = jive_bitsymbolicconstant_node_match_attrs_, /* override */
	.create = jive_bitsymbolicconstant_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_bitsymbolicconstant_node_init_(
	jive_bitsymbolicconstant_node * self,
	jive_region * region,
	size_t nbits, const char * name)
{
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	jive_node_init_(&self->base, region,
		0, NULL, NULL,
		1, &type);
	self->attrs.nbits = nbits;
	size_t len = strlen(name);
	self->attrs.name = jive_context_malloc(region->graph->context, len + 1);
	size_t n;
	for(n=0; n<len+1; n++) self->attrs.name[n] = name[n];
}

static void
jive_bitsymbolicconstant_node_fini_(jive_node * self_)
{
	jive_bitsymbolicconstant_node * self = (jive_bitsymbolicconstant_node *) self_;
	jive_context_free(self->base.graph->context, self->attrs.name);
	jive_node_fini_(&self->base);
}

static void
jive_bitsymbolicconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_bitsymbolicconstant_node * self = (const jive_bitsymbolicconstant_node *) self_;
	jive_buffer_putstr(buffer, self->attrs.name);
}

static const jive_node_attrs *
jive_bitsymbolicconstant_node_get_attrs_(const jive_node * self_)
{
	const jive_bitsymbolicconstant_node * self = (const jive_bitsymbolicconstant_node *) self_;
	return &self->attrs.base;
}

static bool
jive_bitsymbolicconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_bitsymbolicconstant_node_attrs * first = &((const jive_bitsymbolicconstant_node *)self)->attrs;
	const jive_bitsymbolicconstant_node_attrs * second = (const jive_bitsymbolicconstant_node_attrs *) attrs;
	if (first->nbits != second->nbits) return false;
	return strcmp(first->name, second->name) == 0;
}

static jive_node *
jive_bitsymbolicconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_bitsymbolicconstant_node_attrs * attrs = (const jive_bitsymbolicconstant_node_attrs *) attrs_;
	jive_bitsymbolicconstant_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_BITSYMBOLICCONSTANT_NODE;
	jive_bitsymbolicconstant_node_init_(node, region, attrs->nbits, attrs->name);
	return &node->base;
}

jive_node *
jive_bitsymbolicconstant_create(jive_graph * graph, size_t nbits, const char * name)
{
	jive_bitsymbolicconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.name = (char *)name;

	return jive_nullary_operation_create_normalized(&JIVE_BITSYMBOLICCONSTANT_NODE, graph,
		&attrs.base)->node;
}

jive_output *
jive_bitsymbolicconstant(jive_graph * graph, size_t nbits, const char * name)
{
	jive_bitsymbolicconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.name = (char *)name;

	return jive_nullary_operation_create_normalized(&JIVE_BITSYMBOLICCONSTANT_NODE, graph,
		&attrs.base);
}
