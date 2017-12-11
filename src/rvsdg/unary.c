/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/unary.h>

namespace jive {

/* unary normal form */

unary_normal_form::~unary_normal_form() noexcept
{}

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

	const auto & op = static_cast<const jive::unary_op&>(node->operation());

	if (get_reducible()) {
		auto tmp = node->input(0)->origin();
		jive_unop_reduction_path_t reduction = op.can_reduce_operand(tmp);
		if (reduction != jive_unop_reduction_none) {
			node->output(0)->replace(op.reduce_operand(reduction, tmp));
			node->region()->remove_node(node);
			return false;
		}
	}

	return simple_normal_form::normalize_node(node);
}

std::vector<jive::output*>
unary_normal_form::normalized_create(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & arguments) const
{
	JIVE_DEBUG_ASSERT(arguments.size() == 1);

	if (get_mutable() && get_reducible()) {
		const auto & un_op = static_cast<const jive::unary_op&>(op);

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

/* unary operator */

unary_op::~unary_op() noexcept {}

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

jive::node_normal_form *
jive_unary_operation_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	jive::node_normal_form * nf = new jive::unary_normal_form(operator_class, parent, graph);
	
	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::unary_op), jive_unary_operation_get_default_normal_form_);
}
