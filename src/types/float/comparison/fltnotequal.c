/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltnotequal.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

const char fltnotequal_name[] = "FLTNOTEQUAL";

}
}

const jive_node_class JIVE_FLTNOTEQUAL_NODE = {
	parent : &JIVE_FLTCOMPARISON_NODE,
	name : "FLTNOTEQUAL",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_fltnotequal(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::ne_op::normalized_create(arg1, arg2);
}
