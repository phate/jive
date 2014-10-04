/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/node-normal-form.h>

#include <jive/vsdg/graph.h>

namespace jive {

node_normal_form::~node_normal_form() noexcept
{
}

bool
node_normal_form::normalize_node(jive_node * node) const
{
	return true;
}

bool
node_normal_form::operands_are_normalized(
	const jive::operation & op,
	const std::vector<jive::output *> & arguments) const
{
	return true;
}

std::vector<jive::output *>
node_normal_form::normalized_create(
	const jive::operation & op,
	const std::vector<jive::output *> & arguments) const
{
	const jive_node_class * cls = node_class;

	jive_region * region = graph()->root_region;
	if (!arguments.empty()) {
		region = jive_region_innermost(arguments.size(), &arguments[0]);
	}

	jive_node * node = nullptr;
	if (get_mutable() && get_cse()) {
		node = jive_node_cse(region, node_class, &op, arguments.size(), &arguments[0]);
	}

	if (!node) {
		node = jive_node_create(node_class, op, region, arguments.size(), &arguments[0]);
	}

	return std::vector<jive::output *>(&node->outputs[0], &node->outputs[node->noutputs]);
}

void
node_normal_form::set_mutable(bool enable)
{
	if (enable_mutable_ == enable) {
		return;
	}

	children_set<node_normal_form, &node_normal_form::set_mutable>(enable);

	enable_mutable_ = enable;
	if (enable) {
		jive_graph_mark_denormalized(graph());
	}
}

void
node_normal_form::set_cse(bool enable)
{
	if (enable_cse_ == enable) {
		return;
	}

	children_set<node_normal_form, &node_normal_form::set_cse>(enable);

	enable_cse_ = enable;
	if (enable && enable_mutable_) {
		jive_graph_mark_denormalized(graph());
	}
}

}

const jive_node_normal_form_class JIVE_NODE_NORMAL_FORM = {
	parent : 0,
	fini : nullptr,
	normalize_node : nullptr,
	operands_are_normalized : nullptr,
	normalized_create : nullptr,
	set_mutable : nullptr,
	set_cse : nullptr
};
