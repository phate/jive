#include <jive/types/bitstring/bitoperation-classes.h>

#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>

const jive_bitbinary_operation_class JIVE_BITBINARY_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BINARY_OPERATION,
			.name = "BITBINARY",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_node_create_, /* inherit */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		
		.flags = jive_binary_operation_none,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,
		
		.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_ /* inherit */,
		.reduce_operand_pair = jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	.type = jive_bitop_code_invalid
};

/* unary operations */

const jive_bitunary_operation_class JIVE_BITUNARY_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_UNARY_OPERATION,
			.name = "BITUNARY",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_node_create_, /* inherit */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		
		.single_apply_over = NULL,
		.multi_apply_over = NULL,
		
		.can_reduce_operand = jive_unary_operation_can_reduce_operand_ /* inherit */,
		.reduce_operand = jive_unary_operation_reduce_operand_ /* inherit */
	},
	.type = jive_bitop_code_invalid
};

/* bitcomparison_operation_class */

const jive_bitcomparison_operation_class JIVE_BITCOMPARISON_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BINARY_OPERATION,
			.name = "BITCOMPARISON",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_node_create_, /* inherit */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		
		.flags = jive_binary_operation_none,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,
		
		.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_ /* inherit */,
		.reduce_operand_pair = jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	.type = jive_bitcmp_code_invalid
};
