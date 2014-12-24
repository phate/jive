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
namespace bits {

unary_op::~unary_op() noexcept {}

size_t
unary_op::narguments() const noexcept
{
	return 1;
}

const jive::base::type &
unary_op::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
unary_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
unary_op::result_type(size_t index) const noexcept
{
	return type_;
}

jive_binop_reduction_path_t
unary_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	bool arg_is_constant =
		dynamic_cast<const bits::constant_op *>(&arg->node()->operation());
	
	if (arg_is_constant) {
		return jive_unop_reduction_constant;
	}

	return jive_unop_reduction_none;
}

jive::output *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		jive_graph * graph = arg->node()->graph;
		const bits::constant_op & c =
			static_cast<const bits::constant_op&>(arg->node()->operation());
		value_repr result = reduce_constant(c.value());
		return jive_bitconstant(graph, result.nbits(), &result[0]);
	}

	return nullptr;
}


binary_op::~binary_op() noexcept {}

size_t
binary_op::narguments() const noexcept
{
	return arity_;
}

const jive::base::type &
binary_op::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
binary_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
binary_op::result_type(size_t index) const noexcept
{
	return type_;
}

jive_binop_reduction_path_t
binary_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	bool arg1_is_constant =
		dynamic_cast<const bits::constant_op *>(&arg1->node()->operation());
	bool arg2_is_constant =
		dynamic_cast<const bits::constant_op *>(&arg2->node()->operation());
	
	if (arg1_is_constant && arg2_is_constant) {
		return jive_binop_reduction_constants;
	}

	return jive_binop_reduction_none;
}

jive::output *
binary_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const bits::constant_op & c1 =
			static_cast<const bits::constant_op&>(arg1->node()->operation());
		const bits::constant_op & c2 =
			static_cast<const bits::constant_op&>(arg2->node()->operation());
		value_repr result = reduce_constants(c1.value(), c2.value());
		return jive_bitconstant(graph, result.nbits(), &result[0]);
	}

	return nullptr;
}

compare_op::~compare_op() noexcept {}

size_t
compare_op::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
compare_op::argument_type(size_t index) const noexcept
{
	return type_;
}

size_t
compare_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
compare_op::result_type(size_t index) const noexcept
{
	static const jive::bits::type bit(1);
	return bit;
}

jive_binop_reduction_path_t
compare_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	const bits::constant_op * c1_op =
		dynamic_cast<const bits::constant_op *>(&arg1->node()->operation());
	const bits::constant_op * c2_op =
		dynamic_cast<const bits::constant_op *>(&arg2->node()->operation());

	value_repr arg1_repr = c1_op ? c1_op->value() : value_repr(type_.nbits(), 'D');
	value_repr arg2_repr = c2_op ? c2_op->value() : value_repr(type_.nbits(), 'D');

	switch (reduce_constants(arg1_repr, arg2_repr)) {
		case compare_result::static_false:
			return 1;
		case compare_result::static_true:
			return 2;
		case compare_result::undecidable:
			return jive_binop_reduction_none;
	}
	
	return jive_binop_reduction_none;
}

jive::output *
compare_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;
	if (path == 1) {
		return jive_bitconstant(graph, 1, "0");
	}
	if (path == 2) {
		return jive_bitconstant(graph, 1, "1");
	}

	return nullptr;
}

}
}
