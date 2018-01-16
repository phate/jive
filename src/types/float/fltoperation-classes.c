/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/float/fltconstant.h>
#include <jive/types/float/fltoperation-classes.h>

#include <jive/rvsdg/control.h>

namespace jive {
namespace flt {

static const type type_instance;
static const jive::port p(type_instance);

unary_op::~unary_op() noexcept
{}

jive_unop_reduction_path_t
unary_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	if (arg->node() && dynamic_cast<const constant_op*>(&arg->node()->operation()))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::output *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		auto & c = static_cast<const constant_op&>(arg->node()->operation());
		value_repr result = reduce_constant(c.value());
		return jive_fltconstant(arg->region(), result);
	}

	return nullptr;
}

binary_op::~binary_op() noexcept
{
}

size_t
binary_op::narguments() const noexcept
{
	return 2;
}

const jive::port &
binary_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return p;
}

size_t
binary_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
binary_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return p;
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
		auto & c1 = static_cast<const constant_op&>(arg1->node()->operation());
		auto & c2 = static_cast<const constant_op&>(arg2->node()->operation());
		value_repr result = reduce_constants(c1.value(), c2.value());
		return jive_fltconstant(arg1->region(), result);
	}

	return nullptr;
}

compare_op::~compare_op() noexcept
{
}

size_t
compare_op::narguments() const noexcept
{
	return 2;
}

const jive::port &
compare_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return p;
}

size_t
compare_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
compare_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	static const jive::port p(jive::bittype(1));
	return p;
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
		auto & c1 = static_cast<const constant_op&>(arg1->node()->operation());
		auto & c2 = static_cast<const constant_op&>(arg2->node()->operation());
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
