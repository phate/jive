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
	const std::type_info & operator_class,
	const jive_node_class * node_class_old,
	jive::node_normal_form * parent,
	jive_graph * graph) noexcept
	: anchor_normal_form(operator_class, node_class_old, parent, graph)
{
}

}
