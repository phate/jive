/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/binary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/binary-normal-form.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {

binary_op::~binary_op() noexcept {}

jive_binary_operation_flags
binary_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

}
}

/* node class */

const jive_node_class JIVE_BINARY_OPERATION = {
	parent : &JIVE_NODE,
	name : "BINARY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

/* node class inheritable methods */

jive::node_normal_form *
jive_binary_operation_get_default_normal_form_(
	const std::type_info & operator_class,
	const jive_node_class * cls,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive::binary_normal_form * nf = new jive::binary_normal_form(operator_class, cls, parent, graph);

	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::base::binary_op), jive_binary_operation_get_default_normal_form_);
}
