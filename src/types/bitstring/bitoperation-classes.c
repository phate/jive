/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdexcept>

#include <jive/types/bitstring/bitoperation-classes.h>
#include <jive/types/bitstring/constant.h>

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>

namespace jive {

bits_unary_operation::~bits_unary_operation() noexcept {}

size_t
bits_unary_operation::narguments() const noexcept
{
	return 1;
}

const jive_type &
bits_unary_operation::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
bits_unary_operation::nresults() const noexcept
{
	return 1;
}

const jive_type &
bits_unary_operation::result_type(size_t index) const noexcept
{
	return type_;
}

jive_binop_reduction_path_t
bits_unary_operation::can_reduce_operand(
	const jive_output * arg) const noexcept
{
	bool arg_is_constant =
		dynamic_cast<const bitstring::constant_operation *>(&arg->node()->operation());
	
	if (arg_is_constant) {
		return jive_unop_reduction_constant;
	}

	return jive_unop_reduction_none;
}

jive_output *
bits_unary_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive_output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		jive_graph * graph = arg->node()->graph;
		const bitstring::constant_operation & c =
			static_cast<const bitstring::constant_operation&>(arg->node()->operation());
		bitstring::value_repr result = reduce_constant(c.bits);
		return jive_bitconstant(graph, result.size(), &result[0]);
	}

	return nullptr;
}


bits_binary_operation::~bits_binary_operation() noexcept {}

size_t
bits_binary_operation::narguments() const noexcept
{
	return arity_;
}

const jive_type &
bits_binary_operation::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
bits_binary_operation::nresults() const noexcept
{
	return 1;
}

const jive_type &
bits_binary_operation::result_type(size_t index) const noexcept
{
	return type_;
}

jive_binop_reduction_path_t
bits_binary_operation::can_reduce_operand_pair(
	const jive_output * arg1,
	const jive_output * arg2) const noexcept
{
	bool arg1_is_constant =
		dynamic_cast<const bitstring::constant_operation *>(&arg1->node()->operation());
	bool arg2_is_constant =
		dynamic_cast<const bitstring::constant_operation *>(&arg2->node()->operation());
	
	if (arg1_is_constant && arg2_is_constant) {
		return jive_binop_reduction_constants;
	}

	return jive_binop_reduction_none;
}

jive_output *
bits_binary_operation::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive_output * arg1,
	jive_output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const bitstring::constant_operation & c1 =
			static_cast<const bitstring::constant_operation&>(arg1->node()->operation());
		const bitstring::constant_operation & c2 =
			static_cast<const bitstring::constant_operation&>(arg2->node()->operation());
		bitstring::value_repr result = reduce_constants(c1.bits, c2.bits);
		return jive_bitconstant(graph, result.size(), &result[0]);
	}

	return nullptr;
}

bits_compare_operation::~bits_compare_operation() noexcept {}

}

static void
jive_bitoperation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	if (noperands == 0)
		return;

	const jive::bits::output * output = dynamic_cast<const jive::bits::output*>(operands[0]);
	if (!output) {
		char * error_msg = jive_context_strjoin(context, "Type mismatch: ", cls->name,
			"node requires bitstring operands.");
		jive_context_fatal_error(context, error_msg);
	}

	size_t nbits = output->nbits();
	if (nbits == 0)
		jive_context_fatal_error(context,
			"Type mismatch: length of bitstring must be greater than zero.");

	size_t n;
	for (n = 1; n < noperands; n++) {
		output = dynamic_cast<const jive::bits::output*>(operands[n]);
		if (!output) {
			char * error_msg = jive_context_strjoin(context, "Type mismatch: ", cls->name,
				"node requires bitstring operands.");
			jive_context_fatal_error(context, error_msg);
		}

		if (nbits != output->nbits())
			jive_raise_type_error(&operands[0]->type(), &operands[n]->type(), context);
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
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitbinary_operation_check_operands_, /* override */
			create : nullptr,
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
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitunary_operation_check_operands_, /* override */
			create : nullptr,
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
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitcomparison_operation_check_operands_, /* override */
			create : nullptr,
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
