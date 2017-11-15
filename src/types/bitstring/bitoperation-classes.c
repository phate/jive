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

namespace jive {
namespace bits {

unary_op::~unary_op() noexcept {}

size_t
unary_op::narguments() const noexcept
{
	return 1;
}

const jive::port &
unary_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
}

size_t
unary_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
unary_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return port_;
}

jive_binop_reduction_path_t
unary_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	if (is_bitconstant_node(producer(arg)))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::output *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		const auto c = static_cast<const bits::constant_op&>(producer(arg)->operation());
		return create_bitconstant(arg->node()->region(), reduce_constant(c.value()));
	}

	return nullptr;
}


binary_op::~binary_op() noexcept {}

size_t
binary_op::narguments() const noexcept
{
	return arity_;
}

const jive::port &
binary_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
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
	return port_;
}

jive_binop_reduction_path_t
binary_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	if (is_bitconstant_node(producer(arg1)) && is_bitconstant_node(producer(arg2)))
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
		const auto c1 = static_cast<const bits::constant_op&>(producer(arg1)->operation());
		const auto c2 = static_cast<const bits::constant_op&>(producer(arg2)->operation());
		return create_bitconstant(arg1->region(), reduce_constants(c1.value(), c2.value()));
	}

	return nullptr;
}

compare_op::~compare_op() noexcept {}

size_t
compare_op::narguments() const noexcept
{
	return 2;
}

const jive::port &
compare_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
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
	static const jive::port port(jive::bits::type(1));
	return port;
}

jive_binop_reduction_path_t
compare_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	auto p = producer(arg1);
	const bits::constant_op * c1_op = nullptr;
	if (p) c1_op = dynamic_cast<const bits::constant_op*>(&p->operation());

	p = producer(arg2);
	const bits::constant_op * c2_op = nullptr;
	if (p) c2_op = dynamic_cast<const bits::constant_op*>(&p->operation());

	value_repr arg1_repr = c1_op ? c1_op->value() : value_repr::repeat(type().nbits(), 'D');
	value_repr arg2_repr = c2_op ? c2_op->value() : value_repr::repeat(type().nbits(), 'D');

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
	if (path == 1) {
		return create_bitconstant(arg1->region(), "0");
	}
	if (path == 2) {
		return create_bitconstant(arg1->region(), "1");
	}

	return nullptr;
}

}
}
