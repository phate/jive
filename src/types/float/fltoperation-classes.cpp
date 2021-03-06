/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.hpp>
#include <jive/types/bitstring/type.hpp>
#include <jive/types/float/fltconstant.hpp>
#include <jive/types/float/fltoperation-classes.hpp>

#include <jive/rvsdg/control.hpp>

namespace jive {
namespace flt {

unary_op::~unary_op() noexcept
{}

jive_unop_reduction_path_t
unary_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	if (is<constant_op>(node_output::node(arg)))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::output *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		auto node = static_cast<node_output*>(arg)->node();
		auto & c = static_cast<const constant_op&>(node->operation());
		value_repr result = reduce_constant(c.value());
		return jive_fltconstant(arg->region(), result);
	}

	return nullptr;
}

binary_op::~binary_op() noexcept
{
}

/* reduction methods */
jive_binop_reduction_path_t
binary_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	auto op1 = dynamic_cast<const jive::simple_output*>(arg1);
	auto op2 = dynamic_cast<const jive::simple_output*>(arg2);
	if (!op1 || !op2)
		return jive_binop_reduction_none;

	bool arg1_is_constant = dynamic_cast<const constant_op *>(&op1->node()->operation());
	bool arg2_is_constant = dynamic_cast<const constant_op *>(&op2->node()->operation());
	if (arg1_is_constant && arg2_is_constant)
		return jive_binop_reduction_constants;

	return jive_binop_reduction_none;
}

jive::output *
binary_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	if (path == jive_binop_reduction_constants) {
		auto node1 = static_cast<node_output*>(arg1)->node();
		auto node2 = static_cast<node_output*>(arg2)->node();
		auto & c1 = static_cast<const constant_op&>(node1->operation());
		auto & c2 = static_cast<const constant_op&>(node2->operation());
		value_repr result = reduce_constants(c1.value(), c2.value());
		return jive_fltconstant(arg1->region(), result);
	}

	return nullptr;
}

compare_op::~compare_op() noexcept
{
}

jive_binop_reduction_path_t
compare_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	auto op1 = dynamic_cast<const jive::simple_output*>(arg1);
	auto op2 = dynamic_cast<const jive::simple_output*>(arg2);
	if (!op1 || !op2)
		return jive_binop_reduction_none;

	bool arg1_is_constant = dynamic_cast<const constant_op *>(&op1->node()->operation());
	bool arg2_is_constant = dynamic_cast<const constant_op *>(&op2->node()->operation());
	if (arg1_is_constant && arg2_is_constant)
		return jive_binop_reduction_constants;

	return jive_binop_reduction_none;
}

jive::output *
compare_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	if (path == jive_binop_reduction_constants) {
		auto node1 = static_cast<node_output*>(arg1)->node();
		auto node2 = static_cast<node_output*>(arg2)->node();
		auto & c1 = static_cast<const constant_op&>(node1->operation());
		auto & c2 = static_cast<const constant_op&>(node2->operation());
		bool result = reduce_constants(c1.value(), c2.value());
		if (result) {
			return create_bitconstant(arg1->region(), "1");
		} else {
			return create_bitconstant(arg1->region(), "0");
		}
	}

	return nullptr;
}

}
}
