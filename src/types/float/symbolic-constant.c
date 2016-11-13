/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/symbolic-constant.h>

#include <string.h>

#include <jive/types/float/flttype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_symbol_op<jive::flt::type>;
}
}



jive::oport *
jive_fltsymbolicconstant(jive_graph * graph, const char * name)
{
	jive::flt::symbol_op op(name, jive::flt::type());
	return jive_node_create_normalized(graph->root(), op, {})[0];
}
