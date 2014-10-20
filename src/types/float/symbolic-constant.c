/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
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
template class domain_symbol_op<&JIVE_FLTSYMBOLICCONSTANT_NODE, jive::flt::type>;
}
}


const jive_node_class JIVE_FLTSYMBOLICCONSTANT_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "FLTSYMBOLICCONSTANT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};


jive::output *
jive_fltsymbolicconstant(jive_graph * graph, const char * name)
{
	jive::flt::symbol_op op(name, jive::flt::type());

	return jive_node_create_normalized(graph, op, {})[0];
}
