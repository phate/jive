/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltnegate.h>
#include <jive/types/float/fltoperation-classes-private.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

value_repr compute_negation(value_repr arg)
{
	return -arg;
}

const char fltnegate_name[] = "FLTNEGATE";

}
}

const jive_fltunary_operation_class JIVE_FLTNEGATE_NODE_ = {
	base : { /* jive_unary_opeartion_class */
		base : {	/* jive_node_class */
			parent : &JIVE_FLTUNARY_NODE,
			name : "FLTNEGATE",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : nullptr,
			match_attrs : nullptr,
			check_operands : nullptr,
			create : nullptr,
		},
		
		single_apply_over : NULL,
		multi_apply_over : NULL,

		can_reduce_operand : nullptr,
		reduce_operand : nullptr
	},
	type : jive_fltop_code_negate
};

jive::output *
jive_fltnegate(jive::output * arg)
{
	return jive::flt::negate_operation::normalized_create(arg);
}
