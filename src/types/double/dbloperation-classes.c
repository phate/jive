/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/double/dbloperation-classes.h>
#include <jive/types/double/dbloperation-classes-private.h>
#include <jive/types/double/dbltype.h>
#include <jive/vsdg/node-private.h>

static void
jive_dbloperation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	if (noperands == 0)
		return;

	size_t n;
	JIVE_DECLARE_DOUBLE_TYPE(dbltype);
	for (n = 0; n < noperands; n++) {
		if (!jive_double_output_const_cast(operands[n]))
			jive_raise_type_error(dbltype, jive_output_get_type(operands[n]), context);
	}
}

/* dblbinary operation class */

void
jive_dblbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	jive_dbloperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_dblbinary_operation_class JIVE_DBLBINARY_NODE_ = {
	base : {	/* jive_binary_operation_class */
		base : {	/* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "DBLBINARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_dblbinary_operation_check_operands_, /* override */
			create : jive_node_create_, /* inherit */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},

		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},

	type : jive_dblop_code_invalid
};

/* dblunary operation class */

void
jive_dblunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	jive_dbloperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_dblunary_operation_class JIVE_DBLUNARY_NODE_ = {
	base : { /* jive_unary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_UNARY_OPERATION,
			name : "DBLUNARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_dblunary_operation_check_operands_, /* override */
			create : jive_node_create_, /* inherit */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},

		single_apply_over : NULL,
		multi_apply_over : NULL,

		can_reduce_operand : jive_unary_operation_can_reduce_operand_, /* inherit */
		reduce_operand : jive_unary_operation_reduce_operand_ /* inherit */
	},

	type : jive_dblop_code_invalid
};

/* dblcomparison operation class */

void
jive_dblcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context)
{
	jive_dbloperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_dblcomparison_operation_class JIVE_DBLCOMPARISON_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "DBLCOMPARISON",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_dblcomparison_operation_check_operands_, /* override */
			create : jive_node_create_, /* inherit */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},

		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},

	type : jive_dblcmp_code_invalid
};
