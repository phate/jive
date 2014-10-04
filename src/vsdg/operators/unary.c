/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/unary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
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

/* node class */

const jive_node_class JIVE_UNARY_OPERATION = {
	parent : &JIVE_NODE,
	.name ="UNARY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* override */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

/* node class inheritable methods */

jive::node_normal_form *
jive_unary_operation_get_default_normal_form_(
	const jive_node_class * cls,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive_context * context = graph->context;
	jive::node_normal_form * nf = new jive::unary_normal_form(cls, parent, graph);
	
	nf->class_ = &JIVE_UNARY_OPERATION_NORMAL_FORM;
	
	
	return nf;
}
