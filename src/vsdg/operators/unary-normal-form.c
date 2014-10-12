/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/unary-normal-form.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/unary.h>
#include <jive/vsdg/region.h>

namespace jive {

unary_normal_form::~unary_normal_form() noexcept
{
}

unary_normal_form::unary_normal_form(
	const jive_node_class * node_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
	: node_normal_form(node_class, parent, graph)
	, enable_reducible_(true)
{
	if (auto p = dynamic_cast<unary_normal_form *>(parent)) {
		enable_reducible_ = p->enable_reducible_;
	}
}

bool
unary_normal_form::normalize_node(jive_node * node) const
{
	if (!get_mutable()) {
		return true;
	}

	const jive::base::unary_op & op =
		static_cast<const jive::base::unary_op &>(node->operation());

	jive::output * output = node->outputs[0];

	if (get_reducible()) {
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

	if (get_cse()) {
		jive::output * operands[] = { node->inputs[0]->origin() };
		jive_node * new_node = jive_node_cse(node->region, node_class, &op, 1, operands);
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
unary_normal_form::operands_are_normalized(
	const jive::operation & base_op,
	const std::vector<jive::output *> & arguments) const
{
	JIVE_DEBUG_ASSERT(arguments.size() == 1);

	if (get_mutable() && get_reducible()) {
		const jive::base::unary_op & op =
			static_cast<const jive::base::unary_op &>(base_op);

		jive_unop_reduction_path_t reduction =
			op.can_reduce_operand(arguments[0]);
		if (reduction != jive_unop_reduction_none) {
			return false;
		}
	}

	return node_normal_form::operands_are_normalized(base_op, arguments);
}

std::vector<jive::output *>
unary_normal_form::normalized_create(
	const jive::operation & op,
	const std::vector<jive::output *> & arguments) const
{
	JIVE_DEBUG_ASSERT(arguments.size() == 1);

	if (get_mutable() && get_reducible()) {
		const jive::base::unary_op & un_op =
			static_cast<const jive::base::unary_op &>(op);

		jive_unop_reduction_path_t reduction =
			un_op.can_reduce_operand(arguments[0]);
		if (reduction != jive_unop_reduction_none) {
			return { un_op.reduce_operand(reduction, arguments[0]) };
		}
	}

	return node_normal_form::normalized_create(op, arguments);
}

void
unary_normal_form::set_reducible(bool enable)
{
	if (get_reducible() == enable) {
		return;
	}

	children_set<unary_normal_form, &unary_normal_form::set_reducible>(enable);

	enable_reducible_ = enable;
	if (get_mutable() && enable) {
		jive_graph_mark_denormalized(graph());
	}
}

}

const jive_node_normal_form_class JIVE_UNARY_OPERATION_NORMAL_FORM = {
	parent : &JIVE_NODE_NORMAL_FORM,
	fini : nullptr, /* inherit */
	normalize_node : nullptr, /* override */
	operands_are_normalized : nullptr, /* inherit */
	normalized_create : nullptr, /* override */
	set_mutable : nullptr, /* inherit */
	set_cse : nullptr /* inherit */
};