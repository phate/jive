/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/splitnode.h>

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/node-private.h>

#include <string.h>

static void
jive_splitnode_init_(
	jive_splitnode * self,
	jive_region * region,
	const jive_type * in_type,
	jive_output * in_origin,
	const struct jive_resource_class * in_class,
	const jive_type * out_type,
	const struct jive_resource_class * out_class)
{
	self->class_ = &JIVE_SPLITNODE;
	jive_node_init_(self, region,
		1, &in_type, &in_origin,
		1, &out_type);
	
	self->inputs[0]->required_rescls = in_class;
	self->outputs[0]->required_rescls = out_class;
}

static jive_node *
jive_splitnode_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive::split_operation * attrs = (const jive::split_operation *) attrs_;
	jive_node * node = jive_splitnode_create(region,
		jive_resource_class_get_type(attrs->in_class()),
		operands[0],
		attrs->in_class(),
		jive_resource_class_get_type(attrs->out_class()),
		attrs->out_class());
	
	node->inputs[0]->required_rescls = attrs->in_class();
	node->outputs[0]->required_rescls = attrs->out_class();
	
	return node;
}

static bool
jive_splitnode_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_splitnode * self = (const jive_splitnode *) self_;
	const jive::split_operation * attrs = (const jive::split_operation *) attrs_;
	return
		self->operation().in_class() == attrs->in_class() &&
		self->operation().out_class() == attrs->out_class();
}

const jive_node_class JIVE_SPLITNODE = {
	parent : &JIVE_NODE,
	name : "SPLIT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : nullptr,
	match_attrs : jive_splitnode_match_attrs_, /* override */
	check_operands : NULL,
	create : jive_splitnode_create_, /* override */
};

jive_node *
jive_splitnode_create(jive_region * region,
	const jive_type * in_type,
	jive_output * in_origin,
	const struct jive_resource_class * in_class,
	const jive_type * out_type,
	const struct jive_resource_class * out_class)
{
	jive_graph * graph = region->graph;
	
	bool input_is_value = dynamic_cast<jive_value_output*>(in_origin);
	bool output_is_value = dynamic_cast<const jive_value_type*>(out_type) != nullptr;
	
	if (!input_is_value && !output_is_value)
		jive_context_fatal_error(graph->context, "States may not be split by a splitnode");
	
	jive_node_normal_form * nf = jive_graph_get_nodeclass_form(graph , &JIVE_SPLITNODE);
	
	jive_splitnode * self = new jive_splitnode(jive::split_operation(in_class, out_class));
	jive_splitnode_init_(self, region, in_type, in_origin, in_class, out_type, out_class);
	
	if (nf->enable_mutable && nf->enable_cse)
		jive_graph_mark_denormalized(graph);
	
	return self;
}
