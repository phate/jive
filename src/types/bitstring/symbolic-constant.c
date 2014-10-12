/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/symbolic-constant.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_symbol_op<&JIVE_BITSYMBOLICCONSTANT_NODE, jive::bits::type>;
}
}

const jive_node_class JIVE_BITSYMBOLICCONSTANT_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "BITSYMBOLICCONSTANT",
	fini : jive_node_fini_, /* override */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitsymbolicconstant(jive_graph * graph, size_t nbits, const char * name)
{
	jive::bits::symbol_op op(name, jive::bits::type(nbits));

	return jive_nullary_operation_create_normalized(&JIVE_BITSYMBOLICCONSTANT_NODE, graph,
		&op);
}
