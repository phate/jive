#include <jive/arch/sizeof.h>

#include <jive/context.h>
#include <jive/bitstring.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/node-private.h>
#include <jive/arch/memlayout.h>
#include <jive/vsdg/recordtype.h>
#include <jive/vsdg/uniontype.h>

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
	.parent = &JIVE_NODE,
	.name = "SIZEOF",
	.fini = jive_sizeof_node_fini_,	/* override */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_sizeof_node_get_attrs_, /* override */
	.match_attrs = jive_sizeof_node_match_attrs_, /* override */
	.create = jive_sizeof_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_sizeof_node_fini_(jive_node * self_)
{
	jive_sizeof_node * self = (jive_sizeof_node *)self_;
	
	jive_context_free(self_->graph->context, self->attrs.type);
	jive_node_fini_(&self->base);		
}

static const jive_node_attrs *
jive_sizeof_node_get_attrs_(const jive_node * self_)
{
	const jive_sizeof_node * self = (const jive_sizeof_node *)self_;
	
	return &self->attrs.base;
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
	jive_sizeof_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_SIZEOF_NODE;
	
	/* FIXME: either need a "universal" integer type,
	or some way to specify the representation type for the
	sizeof operator */
	JIVE_DECLARE_BITSTRING_TYPE(btype, 32);
	jive_node_init_(&node->base, region,
		0, NULL, NULL,
		1, &btype);
	
	node->attrs.type = (jive_value_type *)jive_type_copy(&type->base, context);
	
	return &node->base;
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
	
	jive_output * new = jive_bitconstant_unsigned(node->base.graph, 32, layout->total_size);
	jive_output_replace(node->base.outputs[0], new);		
}
