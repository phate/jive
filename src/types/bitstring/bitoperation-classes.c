/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/bitoperation-classes.h>

#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>

static void
jive_bitoperation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	if (noperands == 0)
		return;

	const jive_bitstring_output * output = jive_bitstring_output_const_cast(operands[0]);
	if (!output) {
		char * error_msg = jive_context_strjoin(context, "Type mismatch: ", cls->name,
			"node requires bitstring operands.");
		jive_context_fatal_error(context, error_msg);
	}

	size_t nbits = jive_bitstring_output_nbits(output);
	if (nbits == 0)
		jive_context_fatal_error(context,
			"Type mismatch: length of bitstring must be greater than zero.");

	size_t n;
	for (n = 1; n < noperands; n++) {
		output = jive_bitstring_output_const_cast(operands[n]);
		if (!output) {
			char * error_msg = jive_context_strjoin(context, "Type mismatch: ", cls->name,
				"node requires bitstring operands.");
			jive_context_fatal_error(context, error_msg);
		}

		if (nbits != jive_bitstring_output_nbits(output))
			jive_raise_type_error(jive_output_get_type(operands[0]), jive_output_get_type(operands[n]),
				context);
	}
}

/* bitbinary operation class */

void
jive_bitbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	jive_bitoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_bitbinary_operation_class JIVE_BITBINARY_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "BITBINARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitbinary_operation_check_operands_, /* override */
			create : jive_node_create_, /* inherit */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},
		
		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,
		
		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_ /* inherit */,
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	type : jive_bitop_code_invalid
};

/* bitunary operation class */

void
jive_bitunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	jive_bitoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_bitunary_operation_class JIVE_BITUNARY_NODE_ = {
	base : { /* jive_unary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_UNARY_OPERATION,
			name : "BITUNARY",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitunary_operation_check_operands_, /* override */
			create : jive_node_create_, /* inherit */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},
		
		single_apply_over : NULL,
		multi_apply_over : NULL,
		
		can_reduce_operand : jive_unary_operation_can_reduce_operand_ /* inherit */,
		reduce_operand : jive_unary_operation_reduce_operand_ /* inherit */
	},
	type : jive_bitop_code_invalid
};

/* bitcomparison operation class */

void
jive_bitcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context)
{
	jive_bitoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_bitcomparison_operation_class JIVE_BITCOMPARISON_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BINARY_OPERATION,
			name : "BITCOMPARISON",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitcomparison_operation_check_operands_, /* override */
			create : jive_node_create_, /* inherit */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},
		
		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,
		
		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_ /* inherit */,
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	type : jive_bitcmp_code_invalid
};
