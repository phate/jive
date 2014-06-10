/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdexcept>

#include <jive/types/bitstring/bitoperation-classes.h>
#include <jive/types/bitstring/constant.h>

#include <jive/vsdg/control.h>
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

const jive::base::type &
bits_unary_operation::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
bits_unary_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
bits_unary_operation::result_type(size_t index) const noexcept
{
	return type_;
}

jive_binop_reduction_path_t
bits_unary_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	bool arg_is_constant =
		dynamic_cast<const bitstring::constant_operation *>(&arg->node()->operation());
	
	if (arg_is_constant) {
		return jive_unop_reduction_constant;
	}

	return jive_unop_reduction_none;
}

jive::output *
bits_unary_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
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

const jive::base::type &
bits_binary_operation::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
bits_binary_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
bits_binary_operation::result_type(size_t index) const noexcept
{
	return type_;
}

jive_binop_reduction_path_t
bits_binary_operation::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
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

jive::output *
bits_binary_operation::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
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

size_t
bits_compare_operation::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
bits_compare_operation::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
bits_compare_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
bits_compare_operation::result_type(size_t index) const noexcept
{
	static const jive::ctl::type ctl;
	return ctl;
}

jive_binop_reduction_path_t
bits_compare_operation::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	const bitstring::constant_operation * c1_op =
		dynamic_cast<const bitstring::constant_operation *>(&arg1->node()->operation());
	const bitstring::constant_operation * c2_op =
		dynamic_cast<const bitstring::constant_operation *>(&arg2->node()->operation());

	bitstring::value_repr arg1_repr;
	if (c1_op) {
		arg1_repr = c1_op->bits;
	} else {
		arg1_repr = bitstring::value_repr(type_.nbits(), 'D');
	}

	bitstring::value_repr arg2_repr;
	if (c2_op) {
		arg2_repr = c2_op->bits;
	} else {
		arg2_repr = bitstring::value_repr(type_.nbits(), 'D');
	}

	switch (reduce_constants(arg1_repr, arg2_repr)) {
		case compare_result::static_false:
			return 1;
		case compare_result::static_true:
			return 2;
		case compare_result::undecidable:
			return jive_binop_reduction_none;
	}
}

jive::output *
bits_compare_operation::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;
	if (path == 1) {
		return jive_control_false(graph);
	}
	if (path == 2) {
		return jive_control_true(graph);
	}

	return nullptr;
}

}

static void
jive_bitoperation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
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

const jive_node_class JIVE_BITBINARY_NODE = {
	parent : &JIVE_BINARY_OPERATION,
	name : "BITBINARY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

/* bitunary operation class */

const jive_node_class JIVE_BITUNARY_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "BITUNARY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

/* bitcomparison operation class */

const jive_node_class JIVE_BITCOMPARISON_NODE = {
	parent : &JIVE_BINARY_OPERATION,
	name : "BITCOMPARISON",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};
