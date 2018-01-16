/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdexcept>

#include <jive/types/bitstring/bitoperation-classes.h>
#include <jive/types/bitstring/constant.h>

#include <jive/rvsdg/control.h>

namespace jive {

/* bitunary operation */

bitunary_op::~bitunary_op() noexcept
{}

size_t
bitunary_op::narguments() const noexcept
{
	return 1;
}

const jive::port &
bitunary_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
}

size_t
bitunary_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
bitunary_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return port_;
}

jive_binop_reduction_path_t
bitunary_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	if (is_bitconstant_node(producer(arg)))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::output *
bitunary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		auto & c = static_cast<const bitconstant_op&>(producer(arg)->operation());
		return create_bitconstant(arg->node()->region(), reduce_constant(c.value()));
	}

	return nullptr;
}

/* bitbinary operation */

bitbinary_op::~bitbinary_op() noexcept
{}

size_t
bitbinary_op::narguments() const noexcept
{
	return arity_;
}

const jive::port &
bitbinary_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
}

size_t
bitbinary_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
bitbinary_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return port_;
}

jive_binop_reduction_path_t
bitbinary_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	if (is_bitconstant_node(producer(arg1)) && is_bitconstant_node(producer(arg2)))
		return jive_binop_reduction_constants;

	return jive_binop_reduction_none;
}

jive::output *
bitbinary_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	if (path == jive_binop_reduction_constants) {
		auto & c1 = static_cast<const bitconstant_op&>(producer(arg1)->operation());
		auto & c2 = static_cast<const bitconstant_op&>(producer(arg2)->operation());
		return create_bitconstant(arg1->region(), reduce_constants(c1.value(), c2.value()));
	}

	return nullptr;
}

/* bitcompare operation */

bitcompare_op::~bitcompare_op() noexcept
{}

size_t
bitcompare_op::narguments() const noexcept
{
	return 2;
}

const jive::port &
bitcompare_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
}

size_t
bitcompare_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
bitcompare_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	static const jive::port port(bittype(1));
	return port;
}

jive_binop_reduction_path_t
bitcompare_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	auto p = producer(arg1);
	const bitconstant_op * c1_op = nullptr;
	if (p) c1_op = dynamic_cast<const bitconstant_op*>(&p->operation());

	p = producer(arg2);
	const bitconstant_op * c2_op = nullptr;
	if (p) c2_op = dynamic_cast<const bitconstant_op*>(&p->operation());

	bitvalue_repr arg1_repr = c1_op ? c1_op->value() : bitvalue_repr::repeat(type().nbits(), 'D');
	bitvalue_repr arg2_repr = c2_op ? c2_op->value() : bitvalue_repr::repeat(type().nbits(), 'D');

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
bitcompare_op::reduce_operand_pair(
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
