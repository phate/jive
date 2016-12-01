/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/unary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators/unary-normal-form.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {

unary_op::~unary_op() noexcept {}

size_t
unary_op::narguments() const noexcept
{
	return 1;
}

size_t
unary_op::nresults() const noexcept
{
	return 1;
}

}
}

jive::node_normal_form *
jive_unary_operation_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive::node_normal_form * nf = new jive::unary_normal_form(operator_class, parent, graph);
	
	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::base::unary_op), jive_unary_operation_get_default_normal_form_);
}
