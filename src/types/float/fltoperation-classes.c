/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/fltoperation-classes-private.h>
#include <jive/types/float/fltoperation-classes.h>
#include <jive/types/float/flttype.h>

#include <jive/vsdg/node-private.h>

namespace jive {

flt_unary_operation::~flt_unary_operation() noexcept
{
}

flt_binary_operation::~flt_binary_operation() noexcept
{
}

flt_compare_operation::~flt_compare_operation() noexcept
{
}

}


static void
jive_fltoperation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	if (noperands == 0)
		return;

	size_t n;
	jive::flt::type flttype;
	for (n = 0; n < noperands; n++) {
		if (!dynamic_cast<const jive::flt::output*>(operands[n]))
			jive_raise_type_error(&flttype, &operands[n]->type(), context);
	}
}

/* fltbinary operation class */

void
jive_fltbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	jive_fltoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_fltbinary_operation_class JIVE_FLTBINARY_NODE_ = {
	base : {	/* jive_binary_operation_class */
		base : {	/* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "FLTBINARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltbinary_operation_check_operands_, /* override */
			create : nullptr,
		},
	
		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	type : jive_fltop_code_invalid
};

/* fltunary operation class */

void
jive_fltunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	jive_fltoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_fltunary_operation_class JIVE_FLTUNARY_NODE_ = {
	base : { /* jive_unary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_UNARY_OPERATION,
			name : "FLTUNARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltunary_operation_check_operands_, /* override */
			create : nullptr,
		},
	
		single_apply_over : NULL,
		multi_apply_over : NULL,

		can_reduce_operand : jive_unary_operation_can_reduce_operand_, /* inherit */
		reduce_operand : jive_unary_operation_reduce_operand_ /* inherit */
	},
	type : jive_fltop_code_invalid
};

/* fltcomparison operation class */

void
jive_fltcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive::output * const operands[],
	jive_context * context)
{
	jive_fltoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_fltcomparison_operation_class JIVE_FLTCOMPARISON_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "FLTCOMPARISON",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltcomparison_operation_check_operands_, /* override */
			create : nullptr,
		},

		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	type : jive_fltcmp_code_invalid
};
