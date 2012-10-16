/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdgroup.h>

#include <string.h>

#include <jive/vsdg/node-private.h>

static void
jive_group_node_init_(jive_group_node * self,
	struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
jive_group_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static char *
jive_group_node_get_label_(const jive_node * self_);

static const jive_node_attrs *
jive_group_node_get_attrs_(const jive_node * self);

static bool
jive_group_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

const jive_node_class JIVE_GROUP_NODE = {
	.parent = &JIVE_NODE,
	.name = "GROUP",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_group_node_get_label_, /* override */
	.get_attrs = jive_group_node_get_attrs_, /* override */
	.match_attrs = jive_group_node_match_attrs_, /* override */
	.create = jive_group_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static char *
jive_group_node_get_label_(const jive_node * self_)
{
	return strdup("GROUP");
}

static const jive_node_attrs *
jive_group_node_get_attrs_(const jive_node * self_)
{
	const jive_group_node * self = (const jive_group_node*)self_;

	return &self->attrs.base;
}

static bool
jive_group_node_match_attrs_(const jive_node * self, const jive_node_attrs * second_)
{
	const jive_group_node_attrs * first = (const jive_group_node_attrs *)jive_node_get_attrs(self);
	const jive_group_node_attrs * second = (const jive_group_node_attrs *)second_;
	
	return first->decl == second->decl;
}

static jive_node *
jive_group_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_group_node_attrs * attrs = (const jive_group_node_attrs *)attrs_ ;

	return jive_group_node_create(region, attrs->decl, noperands, operands);
}

static void
jive_group_node_init_(jive_group_node * self,
	struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[])
{
	if (decl->nelements != narguments) {
		jive_context_fatal_error(region->graph->context,
			"Type mismatch: number of parameters to group does not match record declaration");
	}

	size_t n;
	const jive_type * arg_types[narguments];
	for(n = 0; n < narguments; n++) {
		arg_types[n] = &decl->elements[n]->base;
	}

	jive_record_type type;
	jive_record_type_init(&type, decl);
	const jive_type * rtype = &type.base.base ;

	jive_node_init_(&self->base, region,
		narguments, arg_types, arguments,
		1, &rtype);

	type.base.base.class_->fini(&type.base.base);

	self->attrs.decl = decl;
}

jive_node *
jive_group_node_create(struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[])
{
	jive_group_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_GROUP_NODE;
	jive_group_node_init_(node, region, decl, narguments, arguments);

	return &node->base;
}

jive_output *
jive_group_create(const jive_record_declaration * decl,
	size_t narguments, jive_output * arguments[const])
{
	jive_region * region = jive_region_innermost(narguments, arguments);

	jive_group_node * node = (jive_group_node *)
		jive_group_node_create(region, decl, narguments, arguments);
	
	return node->base.outputs[0];
}

jive_output *
jive_empty_group_create(struct jive_graph * graph, const jive_record_declaration * decl)
{
	return jive_group_node_create(graph->root_region, decl, 0, NULL)->outputs[0];
}
