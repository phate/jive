/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/integral/itgoperation-classes.h>
#include <jive/types/integral/itgoperation-classes-private.h>
#include <jive/types/integral/itgtype.h>

#include <jive/vsdg/node-private.h>

static void
jive_itgoperation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	if (noperands == 0)
		return;

	size_t n;
	JIVE_DECLARE_INTEGRAL_TYPE(itgtype);
	for (n = 0; n < noperands; n++) {
		if (!jive_integral_output_const_cast(operands[n]))
			jive_raise_type_error(itgtype, jive_output_get_type(operands[n]), context);
	}
}

/* itgunary operations */

void
jive_itgunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	jive_itgoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_itgunary_operation_class JIVE_ITGUNARY_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_UNARY_OPERATION,
			.name = "ITGUNARY",
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
		.can_reduce_operand = jive_unary_operation_can_reduce_operand_, /* inherit */
		.reduce_operand = jive_unary_operation_reduce_operand_ /* inherit */
	},
	.type = jive_itgop_code_invalid
};

/* itgbinary operation */

void
jive_itgbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	jive_itgoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_itgbinary_operation_class JIVE_ITGBINARY_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BINARY_OPERATION,
			.name = "ITGBINARY",
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
		.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		.reduce_operand_pair = jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	.type = jive_itgop_code_invalid
};

/* itgcomparison operations */

void
jive_itgcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context)
{
	jive_itgoperation_check_operands_(cls, attrs, noperands, operands, context);
}

const jive_itgcomparison_operation_class JIVE_ITGCOMPARISON_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BINARY_OPERATION,
			.name = "ITGCOMPARISON",
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
		.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		.reduce_operand_pair = jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	.type = jive_itgcmp_code_invalid
};
