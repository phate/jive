/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/anchor.h>

#include <jive/vsdg/anchor-normal-form.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>

namespace jive {

bool
region_anchor_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

size_t
region_anchor_op::narguments() const noexcept
{
	return 1;
}

const jive::base::type &
region_anchor_op::argument_type(size_t index) const noexcept
{
	static const achr::type type;
	return type;
}

size_t
region_anchor_op::nresults() const noexcept
{
	return 0;
}

const jive::base::type &
region_anchor_op::result_type(size_t index) const noexcept
{
	throw std::logic_error("region_anchor_op has no results");
}

}

/* node class */

/* node class inhertiable methods */

jive::node_normal_form *
jive_anchor_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	jive::anchor_normal_form * nf = new jive::anchor_normal_form(operator_class, parent, graph);

	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::region_anchor_op), jive_anchor_node_get_default_normal_form_);
}
