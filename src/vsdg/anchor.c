#include <jive/vsdg/anchor.h>

#include <jive/vsdg/node-private.h>

/* node class */

const jive_anchor_node_class JIVE_ANCHOR_NODE = {
	.parent = &JIVE_NODE,
	.name = "ANCHOR",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_anchor_node_get_default_normal_form_, /* override */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_node_get_attrs_, /* inherit */
	.match_attrs = jive_node_match_attrs_, /* inherit */
	.create = jive_node_create_, /* inherit */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

/* node class inhertiable methods */

jive_node_normal_form *
jive_anchor_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_anchor_node_normal_form * nf = jive_context_malloc(graph->context, sizeof(*nf));
	nf->base.class_ = &JIVE_ANCHOR_NODE_NORMAL_FORM;

	jive_anchor_node_normal_form_init_(nf, cls, parent_, graph);

	return &nf->base;
}

/* normal form class */

const jive_anchor_node_normal_form_class JIVE_ANCHOR_NODE_NORMAL_FORM_ = {
	.base = { /* jive_node_normal_form_class */
		.parent = &JIVE_NODE_NORMAL_FORM,
		.fini = jive_node_normal_form_fini_, /* inherit */
		.normalize_node = jive_node_normal_form_normalize_node_, /* inherit */
		.operands_are_normalized = jive_node_normal_form_operands_are_normalized_, /* inherit */
		.set_mutable = jive_node_normal_form_set_mutable_, /* inherit */
		.set_cse = jive_node_normal_form_set_cse_, /* inherit */
	},
	.set_reducible = jive_anchor_node_normal_form_set_reducible_
};

void
jive_anchor_node_normal_form_init_(jive_anchor_node_normal_form * self,
	const jive_node_class * cls, jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_node_normal_form_init_(&self->base, cls, parent_, graph);
	
	jive_anchor_node_normal_form * parent = jive_anchor_node_normal_form_cast(parent_);

	if (parent)
		self->enable_reducible = parent->enable_reducible;
	else
		self->enable_reducible = true;
}

void
jive_anchor_node_normal_form_set_reducible_(jive_anchor_node_normal_form * self, bool enable)
{
	if (self->enable_reducible == enable)
		return;

	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list)
		jive_anchor_node_normal_form_set_reducible_((jive_anchor_node_normal_form *)child, enable); 
	
	self->enable_reducible = enable;
	if (self->base.enable_mutable && self->enable_reducible)
		jive_graph_mark_denormalized(self->base.graph);	
}
