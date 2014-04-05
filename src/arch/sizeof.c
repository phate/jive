/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/sizeof.h>

#include <jive/arch/memlayout.h>
#include <jive/context.h>
#include <jive/types/bitstring.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/valuetype.h>

/* sizeof node */

static jive_node *
jive_sizeof_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static const jive_node_attrs *
jive_sizeof_node_get_attrs_(const jive_node * self);

static bool
jive_sizeof_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

static void
jive_sizeof_node_fini_(jive_node * self_);

const jive_node_class JIVE_SIZEOF_NODE = {
	parent : &JIVE_NODE,
	name : "SIZEOF",
	fini : jive_sizeof_node_fini_,	/* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_sizeof_node_get_attrs_, /* override */
	match_attrs : jive_sizeof_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_sizeof_node_create_, /* override */
};

static void
jive_sizeof_node_fini_(jive_node * self_)
{
	jive_sizeof_node * self = (jive_sizeof_node *)self_;
	
	jive_context_free(self_->graph->context, self->attrs.type);
	jive_node_fini_(self);
}

static const jive_node_attrs *
jive_sizeof_node_get_attrs_(const jive_node * self_)
{
	const jive_sizeof_node * self = (const jive_sizeof_node *)self_;
	
	return &self->attrs;
}

static bool
jive_sizeof_node_match_attrs_(const jive_node * self, const jive_node_attrs * second_)
{
	const jive_sizeof_node_attrs * first = (const jive_sizeof_node_attrs *)jive_node_get_attrs(self);
	const jive_sizeof_node_attrs * second = (const jive_sizeof_node_attrs *)second_;
	
	return jive_type_equals(&first->type->base, &second->type->base);
}

static jive_node *
jive_sizeof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_sizeof_node_attrs * attrs = (const jive_sizeof_node_attrs *)attrs_;
	
	return jive_sizeof_node_create(region, attrs->type);
}

jive_node *
jive_sizeof_node_create(jive_region * region,
	const jive_value_type * type)
{
	jive_context * context = region->graph->context;
	jive_sizeof_node * node = new jive_sizeof_node;
	
	node->class_ = &JIVE_SIZEOF_NODE;
	
	/* FIXME: either need a "universal" integer type,
	or some way to specify the representation type for the
	sizeof operator */
	JIVE_DECLARE_BITSTRING_TYPE(btype, 32);
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &btype);
	
	node->attrs.type = (jive_value_type *)jive_type_copy(&type->base, context);
	
	return node;
}

jive_output *
jive_sizeof_create(jive_region * region,
	const jive_value_type * type)
{
	return jive_sizeof_node_create(region, type)->outputs[0];
}

/* sizeof reduce */

void
jive_sizeof_node_reduce(const jive_sizeof_node * node, jive_memlayout_mapper * mapper)
{
	const jive_dataitem_memlayout * layout = jive_memlayout_mapper_map_value_type(mapper,
		node->attrs.type);
	
	jive_output * new_node = jive_bitconstant_unsigned(node->graph, 32, layout->total_size);
	jive_output_replace(node->outputs[0], new_node);
}
