/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctsymbolic.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/substitution.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_symbol_op<&JIVE_SYMBOLICFUNCTION_NODE, jive::fct::type>;
}
}

const jive_node_class JIVE_SYMBOLICFUNCTION_NODE = {
	parent : &JIVE_NODE,
	name : "SYMBOLICFUNCTION",
	fini : jive_node_fini_,
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_symbolicfunction_create(
	jive_graph * graph, const char * name, const jive::fct::type * type)
{
	jive::fct::symbol_op op(name, jive::fct::type(*type));

	return jive_nullary_operation_create_normalized(&JIVE_SYMBOLICFUNCTION_NODE, graph,
		&op);
}
