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
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
	: node_normal_form(operator_class, parent, graph)
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

	jive::output * output = node->output(0);

	if (get_reducible()) {
		jive::output * tmp = dynamic_cast<jive::output*>(node->input(0)->origin());
		jive_unop_reduction_path_t reduction = op.can_reduce_operand(tmp);
		if (reduction != jive_unop_reduction_none) {
			output->replace(op.reduce_operand(reduction, tmp));
			/* FIXME: not sure whether "destroy" is really appropriate? */
			delete node;
			return false;
		}
	}

	if (get_cse()) {
		jive_node * new_node = jive_node_cse(node->region(), op,
			{dynamic_cast<jive::output*>(node->input(0)->origin())});
		JIVE_DEBUG_ASSERT(new_node);
		if (new_node != node) {
			output->replace(new_node->output(0));
			/* FIXME: not sure whether "destroy" is really appropriate? */
			delete node;
			return false;
		}
	}

	return true;
}

bool
unary_normal_form::operands_are_normalized(
	const jive::operation & base_op,
	const std::vector<jive::oport*> & arguments) const
{
	JIVE_DEBUG_ASSERT(arguments.size() == 1);

	if (get_mutable() && get_reducible()) {
		const jive::base::unary_op & op =
			static_cast<const jive::base::unary_op &>(base_op);

		jive_unop_reduction_path_t reduction =
			op.can_reduce_operand(dynamic_cast<jive::output*>(arguments[0]));
		if (reduction != jive_unop_reduction_none) {
			return false;
		}
	}

	return node_normal_form::operands_are_normalized(base_op, arguments);
}

std::vector<jive::output *>
unary_normal_form::normalized_create(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments) const
{
	JIVE_DEBUG_ASSERT(arguments.size() == 1);

	if (get_mutable() && get_reducible()) {
		const jive::base::unary_op & un_op =
			static_cast<const jive::base::unary_op &>(op);

		jive_unop_reduction_path_t reduction =
			un_op.can_reduce_operand(dynamic_cast<jive::output*>(arguments[0]));
		if (reduction != jive_unop_reduction_none) {
			return { un_op.reduce_operand(reduction, dynamic_cast<jive::output*>(arguments[0])) };
		}
	}

	return node_normal_form::normalized_create(region, op, arguments);
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
