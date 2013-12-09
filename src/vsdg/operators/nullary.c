/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/nullary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

/* node class */

const jive_node_class JIVE_NULLARY_OPERATION = {
	.parent = &JIVE_NODE,
	.name = "NULLARY",
	.fini = jive_node_fini_,
	.get_default_normal_form = jive_nullary_operation_get_default_normal_form_,
	.get_label = jive_node_get_label_,
	.get_attrs = jive_node_get_attrs_,
	.match_attrs = jive_node_match_attrs_,
	.check_operands = NULL,
	.create = jive_node_create_,
	.get_aux_rescls = jive_node_get_aux_rescls_
};

/* node class inheritable methods */

jive_node_normal_form *
jive_nullary_operation_get_default_normal_form_(const jive_node_class * cls, jive_node_normal_form * parent, jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_nullary_operation_normal_form * nf = jive_context_malloc(context, sizeof(*nf));
	
	jive_node_normal_form_init_(&nf->base, cls, parent, graph);
	nf->base.class_ = &JIVE_NULLARY_OPERATION_NORMAL_FORM;
	
	return &nf->base;
}

/* normal form class */

const jive_nullary_operation_normal_form_class JIVE_NULLARY_OPERATION_NORMAL_FORM_ = {
	.base = {
		.parent = &JIVE_NODE_NORMAL_FORM,
		.fini = jive_node_normal_form_fini_, /* inherit */
		.normalize_node = jive_nullary_operation_normalize_node_, /* override */
		.operands_are_normalized = jive_node_normal_form_operands_are_normalized_, /* inherit */
		.normalized_create = jive_node_normal_form_normalized_create_, /* inherit */
		.set_mutable = jive_node_normal_form_set_mutable_, /* inherit */
		.set_cse = jive_node_normal_form_set_cse_ /* inherit */
	},
	.normalized_create = jive_nullary_operation_normalized_create_
};

/* normal form class inheritable methods */

bool
jive_nullary_operation_normalize_node_(const jive_node_normal_form * self, jive_node * node)
{
	jive_node * new_node = node;
	
	if (!self->enable_mutable)
		return true;
	
	if (self->enable_cse) {
		new_node = jive_node_cse(node->region, node->class_,
			jive_node_get_attrs(node), 0, NULL);
	}
	
	if (new_node != node) {
		jive_output_replace(node->outputs[0], new_node->outputs[0]);
		/* FIXME: not sure whether "destroy" is really appropriate? */
		jive_node_destroy(node);
	}
	
	return new_node == node;
}

jive_output *
jive_nullary_operation_normalized_create_(const jive_nullary_operation_normal_form * self, struct jive_region * region, const jive_node_attrs * attrs)
{
	jive_node * node = jive_node_cse_create(&self->base, region, attrs, 0, 0);
	return node->outputs[0];
}
