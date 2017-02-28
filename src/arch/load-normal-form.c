/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load-normal-form.h>

#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>

namespace jive {

load_normal_form::~load_normal_form() noexcept
{
}

load_normal_form::load_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph) noexcept
	: node_normal_form(operator_class, parent, graph)
	, enable_reducible_(true)
{
	if (auto p = dynamic_cast<load_normal_form *>(parent)) {
		enable_reducible_ = p->enable_reducible_;
	}
}

static bool
is_matching_store_op(const jive::load_op & l_op, const jive::operation & op)
{
	const jive::store_op * s_op = dynamic_cast<const jive::store_op *>(&op);
	if (!s_op)
		return false;

	return l_op.address_type() == s_op->address_type() && l_op.data_type() == s_op->data_type();
}

static bool is_matching_store_node(
	const jive::load_op & l_op, const jive::oport * address,
	const jive::node * node) {
	return
		is_matching_store_op(l_op, node->operation()) &&
		node->input(0)->origin() == address;
}

bool
load_normal_form::normalize_node(jive::node * node) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(node->operation());
		jive::output * address = dynamic_cast<jive::output*>(node->input(0)->origin());
		jive::node * store_node =
			(node->ninputs() >= 2 &&
				is_matching_store_node(l_op, address,
				dynamic_cast<jive::output*>(node->input(1)->origin())->node())) ?
			dynamic_cast<jive::output*>(node->input(1)->origin())->node() : nullptr;
		for (size_t n = 2; n < node->ninputs(); ++n) {
			if (dynamic_cast<jive::output*>(node->input(n)->origin())->node() != store_node) {
				store_node = nullptr;
			}
		}
		if (store_node) {
			node->output(0)->replace(store_node->input(1)->origin());
			delete node;
			return false;
		}
	}

	return node_normal_form::normalize_node(node);
}

bool
load_normal_form::operands_are_normalized(
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(op);
		jive::oport * address = arguments[0];
		auto arg1 = dynamic_cast<jive::output*>(arguments[1]);
		jive::node * store_node =
			(arg1 && arguments.size() >= 2 && is_matching_store_node(l_op, address, arg1->node())) ?
			arg1->node() : nullptr;
		for (size_t n = 2; n < arguments.size(); ++n) {
			auto argn = dynamic_cast<jive::output*>(arguments[n]);
			if (argn && argn->node() != store_node) {
				store_node = nullptr;
			}
		}
		if (store_node) {
			return false;
		}
	}

	return node_normal_form::operands_are_normalized(op, arguments);
}

std::vector<jive::oport*>
load_normal_form::normalized_create(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::oport*> & args) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(op);
		jive::oport * addr = args[0];
		jive::node * store_node = nullptr;
		if (args.size() >= 2 && args[1]->node() && is_matching_store_node(l_op, addr, args[1]->node()))
			store_node = args[1]->node();

		for (size_t n = 2; n < args.size(); ++n) {
			if (args[n]->node() != store_node) {
				store_node = nullptr;
				break;
			}
		}
		if (store_node) {
			return {store_node->input(1)->origin()};
		}
	}

	return node_normal_form::normalized_create(region, op, args);
}

void
load_normal_form::set_reducible(bool enable)
{
	if (get_reducible() == enable) {
		return;
	}

	children_set<load_normal_form, &load_normal_form::set_reducible>(enable);

	enable_reducible_ = enable;
	if (get_mutable() && enable)
		graph()->mark_denormalized();
}

}
