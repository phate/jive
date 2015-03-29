/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/match.h>

#include <cmath>

namespace jive {

match_op::~match_op() noexcept
{}

match_op::match_op(const jive::bits::type & type, const std::vector<size_t> & constants)
	: base::unary_op()
	, otype_(constants.size()+1)
	, itype_(type)
{
	if (std::pow(2.0, type.nbits()) < constants.size() + 1)
		throw compiler_error("More constants than the input type supports.");

	for (size_t n = 0; n < constants.size(); n++) {
		if (mapping_.find(constants[n]) != mapping_.end())
			throw compiler_error("Constant is not unique.");
		mapping_[constants[n]] = n;
	}
}

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
	if (dynamic_cast<const jive::bits::constant_op*>(&arg->node()->operation()))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

jive::output *
match_op::reduce_operand(jive_unop_reduction_path_t path, jive::output * arg) const
{
	jive_graph * graph = arg->node()->graph;

	if (path == jive_unop_reduction_constant) {
		const jive::bits::constant_op * op;
		op = static_cast<const jive::bits::constant_op*>(&arg->node()->operation());
		uint64_t value = op->value().to_uint();
		if (mapping_.find(value) != mapping_.end())
			return jive_control_constant(graph, nalternatives(), mapping_.at(value));
		else
			return jive_control_constant(graph, nalternatives(), nalternatives()-1);
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
