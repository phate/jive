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
#include <jive/vsdg/substitution.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_symbol_op<jive::fct::type>;
}
}

jive::oport *
jive_symbolicfunction_create(
	jive::region * region, const char * name, const jive::fct::type * type)
{
	jive::fct::symbol_op op(name, jive::fct::type(*type));
	return jive_node_create_normalized(region, op, {})[0];
}
