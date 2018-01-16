/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/simple-normal-form.h>
#include <jive/rvsdg/region.h>

namespace jive {

class nullary_normal_form final : public simple_normal_form {
public:
	virtual
	~nullary_normal_form() noexcept
	{
	}

	nullary_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph)
		: simple_normal_form(operator_class, parent, graph)
	{
	}
};

/* nullary operator */

nullary_op::~nullary_op() noexcept
{}

bool
nullary_op::operator==(const jive::operation & other) const noexcept
{
	auto op = dynamic_cast<const nullary_op*>(&other);
	return op && op->result(0) == result(0);
}

}

namespace {

jive::node_normal_form *
jive_nullary_operation_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	return new jive::nullary_normal_form(operator_class, parent, graph);
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::nullary_op), jive_nullary_operation_get_default_normal_form_);
}

}
