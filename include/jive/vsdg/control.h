/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_CONTROL_H
#define JIVE_VSDG_CONTROL_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>

namespace jive {
namespace ctl {

struct type_of_value {
	type operator()(const value_repr & repr) const
	{
		return type();
	}
};

struct format_value {
	std::string operator()(const value_repr & repr) const
	{
		return repr ? "TRUE" : "FALSE";
	}
};

typedef base::domain_const_op<
	type, value_repr, format_value, type_of_value
> constant_op;

}

namespace base {
// declare explicit instantiation
extern template class domain_const_op<
	ctl::type, ctl::value_repr, ctl::format_value, ctl::type_of_value
>;
}

}

jive::output *
jive_control_false(jive_graph * graph);

jive::output *
jive_control_true(jive_graph * graph);

#endif
