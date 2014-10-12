/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/anchor-normal-form.h>

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {

anchor_normal_form::~anchor_normal_form() noexcept
{
}

anchor_normal_form::anchor_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph) noexcept
	: node_normal_form(operator_class, parent, graph)
	, enable_reducible_(true)
{
	if (auto p = dynamic_cast<anchor_normal_form *>(parent)) {
		enable_reducible_ = p->enable_reducible_;
	}
}

void
anchor_normal_form::set_reducible(bool enable)
{
	if (enable_reducible_ == enable) {
		return;
	}

	children_set<anchor_normal_form, &anchor_normal_form::set_reducible>(enable);
	
	enable_reducible_ = enable;
	if (get_mutable() && get_reducible()) {
		jive_graph_mark_denormalized(graph());
	}
}

}
