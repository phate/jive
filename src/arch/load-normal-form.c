/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load-normal-form.h>

#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/vsdg/graph.h>

namespace jive {

load_normal_form::~load_normal_form() noexcept
{
}

load_normal_form::load_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph) noexcept
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
	const jive::load_op & l_op, const jive::output * address,
	const jive_node * node) {
	return
		is_matching_store_op(l_op, node->operation()) &&
		node->input(0)->origin() == address;
}

bool
load_normal_form::normalize_node(jive_node * node) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(node->operation());
		jive::output * address = node->input(0)->origin();
		jive_node * store_node =
			(node->ninputs() >= 2 &&
				is_matching_store_node(l_op, address, node->input(1)->origin()->node())) ?
			node->input(1)->origin()->node() : nullptr;
		for (size_t n = 2; n < node->ninputs(); ++n) {
			if (node->input(n)->origin()->node() != store_node) {
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
	const std::vector<jive::output *> & arguments) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(op);
		jive::output * address = arguments[0];
		jive_node * store_node =
			(arguments.size() >= 2 && is_matching_store_node(l_op, address, arguments[1]->node())) ?
			arguments[1]->node() : nullptr;
		for (size_t n = 2; n < arguments.size(); ++n) {
			if (arguments[n]->node() != store_node) {
				store_node = nullptr;
			}
		}
		if (store_node) {
			return false;
		}
	}

	return node_normal_form::operands_are_normalized(op, arguments);
}

std::vector<jive::output *>
load_normal_form::normalized_create(
	jive_region * region,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(op);
		jive::output * address = arguments[0];
		jive_node * store_node =
			(arguments.size() >= 2 && is_matching_store_node(l_op, address, arguments[1]->node())) ?
			arguments[1]->node() : nullptr;
		for (size_t n = 2; n < arguments.size(); ++n) {
			if (arguments[n]->node() != store_node) {
				store_node = nullptr;
			}
		}
		if (store_node) {
			return {store_node->input(1)->origin()};
		}
	}

	return node_normal_form::normalized_create(region, op, arguments);
}

void
load_normal_form::set_reducible(bool enable)
{
	if (get_reducible() == enable) {
		return;
	}

	children_set<load_normal_form, &load_normal_form::set_reducible>(enable);

	enable_reducible_ = enable;
	if (get_mutable() && enable) {
		jive_graph_mark_denormalized(graph());
	}
}

}
