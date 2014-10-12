/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltequal.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

const char fltequal_name[] = "FLTEQUAL";

}
}

const jive_node_class JIVE_FLTEQUAL_NODE = {
	parent : &JIVE_FLTCOMPARISON_NODE,
	name : "FLTEQUAL",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_fltequal(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::eq_op::normalized_create(arg1, arg2);
}
