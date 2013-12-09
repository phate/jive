/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/unary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

/* node class */

const jive_unary_operation_class JIVE_UNARY_OPERATION_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_NODE,
		.name ="UNARY",
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* override */
		.get_label = jive_node_get_label_, /* inherit */
		.get_attrs = jive_node_get_attrs_, /* inherit */
		.match_attrs = jive_node_match_attrs_, /* inherit */
		.check_operands = NULL,
		.create = jive_node_create_, /* inherit */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},
	
	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_unary_operation_can_reduce_operand_,
	.reduce_operand = jive_unary_operation_reduce_operand_
};

/* node class inheritable methods */

jive_node_normal_form *
jive_unary_operation_get_default_normal_form_(const jive_node_class * cls, jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_unary_operation_normal_form * nf = jive_context_malloc(context, sizeof(*nf));
	
	jive_node_normal_form_init_(&nf->base, cls, parent_, graph);
	nf->base.class_ = &JIVE_UNARY_OPERATION_NORMAL_FORM;
	
	jive_unary_operation_normal_form * parent = jive_unary_operation_normal_form_cast(parent_);
	
	if (parent)
		nf->enable_reducible = parent->enable_reducible;
	else
		nf->enable_reducible = true;
	
	return &nf->base;
}


jive_unop_reduction_path_t
jive_unary_operation_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * operand)
{
	return jive_unop_reduction_none;
}

jive_output *
jive_unary_operation_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand)
{
	return NULL;
}

/* normal form class */

/* FIXME: change name to jive_unary_operation_normalized_create_ after the other normalized_create
	interface has been removed. */
void
jive_unary_operation_normalized_create_new_(const jive_node_normal_form * self_, jive_graph * graph,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_output * results[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive_unary_operation_normal_form * self = (const jive_unary_operation_normal_form *) self_;

	if (self->base.enable_mutable && self->enable_reducible) {
		jive_unop_reduction_path_t reduction;
		reduction = jive_unary_operation_can_reduce_operand(self, attrs, operands[0]);
		if (reduction != jive_unop_reduction_none) {
			results[0] = jive_unary_operation_reduce_operand(reduction, self, attrs, operands[0]);
			return;
		}
	}

	/* FIXME: test for factoring */

	/* FIXME: test for gamma */

	jive_node_normal_form_normalized_create_(self_, graph, attrs, noperands, operands, results);
}

const jive_unary_operation_normal_form_class JIVE_UNARY_OPERATION_NORMAL_FORM_ = {
	.base = {
		.parent = &JIVE_NODE_NORMAL_FORM,
		.fini = jive_node_normal_form_fini_, /* inherit */
		.normalize_node = jive_unary_operation_normalize_node_, /* override */
		.operands_are_normalized = jive_unary_operation_operands_are_normalized_, /* inherit */
		.normalized_create = jive_unary_operation_normalized_create_new_, /* override */
		.set_mutable = jive_node_normal_form_set_mutable_, /* inherit */
		.set_cse = jive_node_normal_form_set_cse_ /* inherit */
	},
	.set_reducible = jive_unary_operation_set_reducible_,
	.normalized_create = jive_unary_operation_normalized_create_
};

/* normal form inheritable methods */

bool
jive_unary_operation_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_unary_operation_normal_form * self = (const jive_unary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	const jive_node_attrs * attrs = jive_node_get_attrs(node);
	
	jive_output * output = node->outputs[0];
	
	if (self->enable_reducible) {
		jive_output * tmp = node->inputs[0]->origin;
		jive_unop_reduction_path_t reduction = jive_unary_operation_can_reduce_operand(self, attrs, tmp);
		if (reduction != jive_unop_reduction_none) {
			tmp = jive_unary_operation_reduce_operand(reduction, self, attrs, tmp);
			jive_output_replace(output, tmp);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}
	
	if (self->base.enable_cse) {
		jive_output * operands[] = { node->inputs[0]->origin };
		jive_node * new_node = jive_node_cse(node->region, self->base.node_class, attrs, 1, operands);
		JIVE_DEBUG_ASSERT(new_node);
		if (new_node != node) {
			jive_output_replace(output, new_node->outputs[0]);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}
	
	return true;
}

bool
jive_unary_operation_operands_are_normalized_(const jive_node_normal_form * self_, size_t noperands,
	jive_output * const operands[], const jive_node_attrs * attrs)
{
	const jive_unary_operation_normal_form * self = (const jive_unary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	JIVE_DEBUG_ASSERT(noperands == 1);
	
	jive_region * region = operands[0]->node->region;
	const jive_node_class * cls = self->base.node_class;

	jive_unop_reduction_path_t reduction;
	reduction = jive_unary_operation_can_reduce_operand(self, attrs, operands[0]);
	if (self->enable_reducible && (reduction != jive_unop_reduction_none)) {
		return false;
	}
	
	/* FIXME: test for factoring */
	
	/* FIXME: test for gamma */
	
	if (self->base.enable_cse && jive_node_cse(region, cls, attrs, noperands, operands))
		return false;
	
	return true;
}

jive_output *
jive_unary_operation_normalized_create_(const jive_unary_operation_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs, jive_output * operand)
{
	const jive_unary_operation_class * cls = (const jive_unary_operation_class *) self->base.node_class;
	
	if (self->base.enable_mutable && self->enable_reducible) {
		jive_unop_reduction_path_t reduction = jive_unary_operation_can_reduce_operand(self, attrs, operand);
		if (reduction != jive_unop_reduction_none)
			return jive_unary_operation_reduce_operand(reduction, self, attrs, operand);
	}
	
	/* FIXME: test for factoring */
	
	/* FIXME: test for gamma */
	
	if (self->base.enable_mutable && self->base.enable_cse) {
		jive_node * node = jive_node_cse(region, &cls->base, attrs, 1, &operand);
		if (node)
			return node->outputs[0];
	}
	
	return cls->base.create(region, attrs, 1, &operand)->outputs[0];
}

void
jive_unary_operation_set_reducible_(jive_unary_operation_normal_form * self_, bool enable)
{
	jive_unary_operation_normal_form * self = (jive_unary_operation_normal_form *) self_;
	if (self->enable_reducible == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list) {
		jive_unary_operation_set_reducible((jive_unary_operation_normal_form *)child, enable);
	}
	
	self->enable_reducible = enable;
	if (self->base.enable_mutable && self->enable_reducible)
		jive_graph_mark_denormalized(self->base.graph);
}
