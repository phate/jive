/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/phi-normal-form.h>

namespace jive {

phi_normal_form::~phi_normal_form() noexcept
{
}

phi_normal_form::phi_normal_form(
	const jive_node_class * node_class,
	jive::node_normal_form * parent,
	jive_graph * graph) noexcept
	: anchor_normal_form(node_class, parent, graph)
{
}

}

const jive_node_normal_form_class JIVE_PHI_NODE_NORMAL_FORM = {
	parent : &JIVE_ANCHOR_NODE_NORMAL_FORM,
	fini : nullptr, /* inherit */
	normalize_node : nullptr, /* override */
	operands_are_normalized : nullptr, /* override */
	normalized_create : nullptr,
	set_mutable : nullptr, /* inherit */
	set_cse : nullptr
};
