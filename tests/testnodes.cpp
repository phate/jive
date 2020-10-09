/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.hpp"

#include <jive/util/ptr-collection.hpp>

namespace jive {
namespace test {

/* unary operation */

unary_op::~unary_op() noexcept
{}

bool
unary_op::operator==(const jive::operation & other) const noexcept
{
	auto op = dynamic_cast<const unary_op*>(&other);
	return op
	    && op->argument(0) == argument(0)
	    && op->result(0) == result(0);
}

jive_unop_reduction_path_t
unary_op::can_reduce_operand(const jive::output * operand) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * operand) const
{
	return nullptr;
}

std::string
unary_op::debug_string() const
{
	return "UNARY_TEST_NODE";
}

std::unique_ptr<jive::operation>
unary_op::copy() const
{
	return std::unique_ptr<jive::operation>(new unary_op(*this));
}

/* binary operation */

binary_op::~binary_op() noexcept
{}

bool
binary_op::operator==(const jive::operation & other) const noexcept
{
	auto op = dynamic_cast<const binary_op*>(&other);
	return op
	    && op->argument(0) == argument(0)
	    && op->result(0) == result(0);
}

jive_binop_reduction_path_t
binary_op::can_reduce_operand_pair(
	const jive::output * op1,
	const jive::output * op2) const noexcept
{
	return jive_binop_reduction_none;
}

jive::output *
binary_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * op1,
	jive::output * op2) const
{
	return nullptr;
}

enum jive::binary_op::flags
binary_op::flags() const noexcept
{
	return flags_;
}

std::string
binary_op::debug_string() const
{
	return "BINARY_TEST_OP";
}

std::unique_ptr<jive::operation>
binary_op::copy() const
{
	return std::unique_ptr<jive::operation>(new binary_op(*this));
}

/* simple operation */

simple_op::~simple_op() noexcept {}

bool
simple_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const jive::test::simple_op*>(&other);
	if (!op || op->narguments() != narguments() || op->nresults() != nresults())
		return false;

	for (size_t n = 0; n < narguments(); n++) {
		if (argument(n) != op->argument(n))
			return false;
	}

	for (size_t n = 0; n < nresults(); n++) {
		if (result(n) != op->result(n))
			return false;
	}

	return true;
}

std::string
simple_op::debug_string() const
{
	return "SIMPLE_TEST_NODE";
}

std::unique_ptr<jive::operation>
simple_op::copy() const
{
	return std::unique_ptr<jive::operation>(new jive::test::simple_op(*this));
}

/* structural operation */

structural_op::~structural_op() noexcept
{}

std::string
structural_op::debug_string() const
{
	return "STRUCTURAL_TEST_NODE";
}

std::unique_ptr<jive::operation>
structural_op::copy() const
{
	return std::unique_ptr<jive::operation>(new structural_op(*this));
}

}}
