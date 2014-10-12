/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/nullary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {

class nullary_normal_form final : public node_normal_form {
public:
	virtual
	~nullary_normal_form() noexcept
	{
	}

	nullary_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive_graph * graph)
		: node_normal_form(operator_class, parent, graph)
	{
	}
};

nullary_op::~nullary_op() noexcept {}

size_t
nullary_op::narguments() const noexcept
{
	return 0;
}

size_t
nullary_op::nresults() const noexcept
{
	return 1;
}

const type &
nullary_op::argument_type(size_t index) const noexcept
{
	throw std::logic_error("No arguments to nullary operation");
}

}
}

/* node class */

const jive_node_class JIVE_NULLARY_OPERATION = {
	parent : &JIVE_NODE,
	name : "NULLARY",
	fini : jive_node_fini_,
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

/* node class inheritable methods */

jive::node_normal_form *
jive_nullary_operation_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive::node_normal_form * nf = new jive::base::nullary_normal_form(
		operator_class, parent, graph);
	
	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::base::nullary_op), jive_nullary_operation_get_default_normal_form_);
}
