/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltdifference.h>
#include <jive/types/float/fltoperation-classes-private.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

value_repr compute_difference(value_repr arg1, value_repr arg2)
{
	return arg1 - arg2;
}

const char fltdifference_name[] = "FLTDIFFERENCE";

}
}

const jive_fltbinary_operation_class JIVE_FLTDIFFERENCE_NODE_ = {
	base : { /* jive_fltbinary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_FLTBINARY_NODE,
			name : "FLTDIFFERENCE",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : nullptr,
			match_attrs : nullptr,
			check_operands : nullptr,
			create : nullptr,
		},

		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : nullptr,
		reduce_operand_pair : nullptr
	},
	type : jive_fltop_code_difference
};

jive::output *
jive_fltdifference(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::difference_operation::normalized_create(arg1, arg2);
}
