/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTSYMBOLIC_H
#define JIVE_TYPES_FUNCTION_FCTSYMBOLIC_H

#include <string>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

namespace jive {
namespace base {
// declare explicit instantiation
extern template class domain_symbol_op<jive::fct::type>;
}

namespace fct {
typedef base::domain_symbol_op<jive::fct::type>
	symbol_op;
}
}

jive::oport *
jive_symbolicfunction_create(
	jive::region * region, const char * name, const jive::fct::type * type);

#endif
