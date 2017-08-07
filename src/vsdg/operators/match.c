/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/operators/match.h>
#include <jive/vsdg/simple_node.h>

#include <cmath>

namespace jive {

match_op::~match_op() noexcept
{}

match_op::match_op(
	size_t nbits,
	const std::map<uint64_t, uint64_t> & mapping,
	uint64_t default_alternative,
	size_t nalternatives)
	: base::unary_op()
	, otype_(nalternatives)
	, itype_(nbits)
	, default_alternative_(default_alternative)
	, mapping_(mapping)
{}

bool
match_op::operator==(const operation & other) const noexcept
{
	const match_op * op = dynamic_cast<const match_op*>(&other);
	return op && op->itype_ == itype_ && op->otype_ == otype_ && op->mapping_ == mapping_;
}

const jive::base::type &
match_op::argument_type(size_t index) const noexcept
{
	return itype_;
}

const jive::base::type &
match_op::result_type(size_t index) const noexcept
{
	return otype_;
}

jive_unop_reduction_path_t
match_op::can_reduce_operand(const jive::output * arg) const noexcept
{
	if (arg->node() && dynamic_cast<const jive::bits::constant_op*>(&arg->node()->operation()))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::output *
match_op::reduce_operand(jive_unop_reduction_path_t path, jive::output * arg) const
{
	if (path == jive_unop_reduction_constant) {
		auto op = static_cast<const jive::bits::constant_op*>(&arg->node()->operation());
		return jive_control_constant(arg->region(), nalternatives(),
			alternative(op->value().to_uint()));
	}

	return nullptr;
}

std::string
match_op::debug_string() const
{
	return "MATCH";
}

std::unique_ptr<jive::operation>
match_op::copy() const
{
	return std::unique_ptr<jive::operation>(new match_op(*this));
}

}
