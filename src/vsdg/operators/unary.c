/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/unary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {

unary_op::~unary_op() noexcept {}

jive_unop_reduction_path_t
unary_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	return nullptr;
}

size_t
unary_op::narguments() const noexcept
{
	return 1;
}

size_t
unary_op::nresults() const noexcept
{
	return 1;
}

}
}

/* node class */

const jive_node_class JIVE_UNARY_OPERATION = {
	parent : &JIVE_NODE,
	.name ="UNARY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* override */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

/* node class inheritable methods */

jive_node_normal_form *
jive_unary_operation_get_default_normal_form_(
	const jive_node_class * cls,
	jive_node_normal_form * parent_,
	jive_graph * graph)
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

/* normal form class */

/* FIXME: change name to jive_unary_operation_normalized_create_ after the other normalized_create
	interface has been removed. */
void
jive_unary_operation_normalized_create_new_(
	const jive_node_normal_form * self_,
	jive_graph * graph,
	const jive::operation * gen_op,
	size_t narguments,
	jive::output * const arguments[],
	jive::output * results[])
{
	JIVE_DEBUG_ASSERT(narguments == 1);

	const jive_unary_operation_normal_form * self = (const jive_unary_operation_normal_form *) self_;

	const jive::base::unary_op & op =
		static_cast<const jive::base::unary_op &>(*gen_op);

	if (self->base.enable_mutable && self->enable_reducible) {
		jive_unop_reduction_path_t reduction;
		reduction = op.can_reduce_operand(arguments[0]);
		if (reduction != jive_unop_reduction_none) {
			results[0] = op.reduce_operand(reduction, arguments[0]);
			return;
		}
	}

	/* FIXME: test for factoring */

	/* FIXME: test for gamma */

	jive_node_normal_form_normalized_create_(self_, graph, &op, narguments, arguments, results);
}

const jive_unary_operation_normal_form_class JIVE_UNARY_OPERATION_NORMAL_FORM_ = {
	base : {
		parent : &JIVE_NODE_NORMAL_FORM,
		fini : jive_node_normal_form_fini_, /* inherit */
		normalize_node : jive_unary_operation_normalize_node_, /* override */
		operands_are_normalized : jive_unary_operation_operands_are_normalized_, /* inherit */
		normalized_create : jive_unary_operation_normalized_create_new_, /* override */
		set_mutable : jive_node_normal_form_set_mutable_, /* inherit */
		set_cse : jive_node_normal_form_set_cse_ /* inherit */
	},
	set_reducible : jive_unary_operation_set_reducible_,
};

/* normal form inheritable methods */

bool
jive_unary_operation_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_unary_operation_normal_form * self = (const jive_unary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	const jive::base::unary_op & op =
		static_cast<const jive::base::unary_op &>(node->operation());
	
	jive::output * output = node->outputs[0];
	
	if (self->enable_reducible) {
		jive::output * tmp = node->inputs[0]->origin();
		jive_unop_reduction_path_t reduction = op.can_reduce_operand(tmp);
		if (reduction != jive_unop_reduction_none) {
			tmp = op.reduce_operand(reduction, tmp);
			jive_output_replace(output, tmp);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}
	
	if (self->base.enable_cse) {
		jive::output * operands[] = { node->inputs[0]->origin() };
		jive_node * new_node = jive_node_cse(node->region, self->base.node_class, &op, 1, operands);
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
jive_unary_operation_operands_are_normalized_(
	const jive_node_normal_form * self_,
	size_t narguments,
	jive::output * const arguments[],
	const jive::operation * gen_op)
{
	const jive_unary_operation_normal_form * self = (const jive_unary_operation_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;
	
	const jive::base::unary_op & op =
		static_cast<const jive::base::unary_op &>(*gen_op);
	
	JIVE_DEBUG_ASSERT(narguments == 1);
	
	jive_region * region = arguments[0]->node()->region;
	const jive_node_class * cls = self->base.node_class;

	jive_unop_reduction_path_t reduction;
	reduction = op.can_reduce_operand(arguments[0]);
	if (self->enable_reducible && (reduction != jive_unop_reduction_none)) {
		return false;
	}
	
	/* FIXME: test for factoring */
	
	/* FIXME: test for gamma */
	
	if (self->base.enable_cse && jive_node_cse(region, cls, &op, narguments, arguments))
		return false;
	
	return true;
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
