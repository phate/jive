/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/unary-normal-form.h>
#include <jive/vsdg/unary.h>

namespace jive {

unary_normal_form::~unary_normal_form() noexcept
{
}

unary_normal_form::unary_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
	: simple_normal_form(operator_class, parent, graph)
	, enable_reducible_(true)
{
	if (auto p = dynamic_cast<unary_normal_form *>(parent)) {
		enable_reducible_ = p->enable_reducible_;
	}
}

bool
unary_normal_form::normalize_node(jive::node * node) const
{
	if (!get_mutable()) {
		return true;
	}

	const jive::base::unary_op & op =
		static_cast<const jive::base::unary_op &>(node->operation());

	if (get_reducible()) {
		auto tmp = node->input(0)->origin();
		jive_unop_reduction_path_t reduction = op.can_reduce_operand(tmp);
		if (reduction != jive_unop_reduction_none) {
			node->output(0)->replace(op.reduce_operand(reduction, tmp));
			node->region()->remove_node(node);
			return false;
		}
	}

	if (get_cse()) {
		jive::node * new_node = jive_node_cse(node->region(), op, {node->input(0)->origin()});
		JIVE_DEBUG_ASSERT(new_node);
		if (new_node != node) {
			node->output(0)->replace(new_node->output(0));
			node->region()->remove_node(node);
			return false;
		}
	}

	return true;
}

std::vector<jive::output*>
unary_normal_form::normalized_create(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & arguments) const
{
	JIVE_DEBUG_ASSERT(arguments.size() == 1);

	if (get_mutable() && get_reducible()) {
		const jive::base::unary_op & un_op =
			static_cast<const jive::base::unary_op &>(op);

		jive_unop_reduction_path_t reduction = un_op.can_reduce_operand(arguments[0]);
		if (reduction != jive_unop_reduction_none) {
			return {un_op.reduce_operand(reduction, arguments[0])};
		}
	}

	return simple_normal_form::normalized_create(region, op, arguments);
}

void
unary_normal_form::set_reducible(bool enable)
{
	if (get_reducible() == enable) {
		return;
	}

	children_set<unary_normal_form, &unary_normal_form::set_reducible>(enable);

	enable_reducible_ = enable;
	if (get_mutable() && enable)
		graph()->mark_denormalized();
}

}
