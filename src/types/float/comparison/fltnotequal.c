/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltnotequal.h>
#include <jive/types/float/fltoperation-classes-private.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

bool compute_notequal(value_repr arg1, value_repr arg2)
{
	return arg1 != arg2;
}

const char fltnotequal_name[] = "FLTNOTEQUAL";

}
}

const jive_fltcomparison_operation_class JIVE_FLTNOTEQUAL_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_FLTCOMPARISON_NODE,
			name : "FLTNOTEQUAL",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : nullptr,
			match_attrs : nullptr,
			check_operands : nullptr,
			create : nullptr,
		},
		flags : jive_binary_operation_commutative,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : nullptr,
		reduce_operand_pair : nullptr,
	},
	type : jive_fltcmp_code_notequal
};

jive::output *
jive_fltnotequal(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::notequal_operation::normalized_create(arg1, arg2);
}
