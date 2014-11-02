/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/control.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_const_op<
	ctl::type, ctl::value_repr, ctl::format_value, ctl::type_of_value
>;
}
}

jive::output *
jive_control_false(jive_graph * graph)
{
	jive::ctl::constant_op op(false);
	return jive_node_create_normalized(graph, op, {})[0];
}

jive::output *
jive_control_true(jive_graph * graph)
{
	jive::ctl::constant_op op(true);
	return jive_node_create_normalized(graph, op, {})[0];
}

