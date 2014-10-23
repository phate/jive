/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/concat.h>

#include <string.h>

#include <jive/common.h>

#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/slice.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

jive::output *
jive_bitconcat(size_t narguments, jive::output * const * arguments)
{
	std::vector<jive::bits::type> types;
	for (size_t n = 0; n < narguments; ++n) {
		types.push_back(dynamic_cast<const jive::bits::type &>(arguments[n]->type()));
	}

	jive_graph * graph = arguments[0]->node()->graph;

	jive::bits::concat_op op(std::move(types));
	return jive_node_create_normalized(
		graph, op, std::vector<jive::output *>(arguments, arguments + narguments))[0];
}

namespace jive {
namespace bits {

type
concat_op::aggregate_arguments(const std::vector<type>& argument_types) noexcept
{
	size_t total = 0;
	for (const type & t : argument_types) {
		total += t.nbits();
	}
	return type(total);
}

concat_op::~concat_op() noexcept
{
}

bool
concat_op::operator==(const operation & other) const noexcept
{
	const concat_op * op = dynamic_cast<const concat_op *>(&other);
	return op && op->argument_types_ == argument_types_;
}

size_t
concat_op::narguments() const noexcept
{
	return argument_types_.size();
}

const jive::base::type &
concat_op::argument_type(size_t index) const noexcept
{
	return argument_types_[index];
}

size_t
concat_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
concat_op::result_type(size_t index) const noexcept
{
	return result_type_;
}

jive_node *
concat_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	std::vector<type> types;
	for (size_t n = 0; n < narguments; ++n) {
		types.push_back(dynamic_cast<const type &>(arguments[n]->type()));
	}

	// FIXME: create new temporary concat_op instance that may have
	// different signature. This is due to concat operation currently
	// being modeled as a "binary operation" which cannot deal with
	// changes of function signature during normalization.
	return jive_opnode_create(
		concat_op(std::move(types)),
		region,
		arguments, arguments + narguments);
}

jive_binop_reduction_path_t
concat_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	const constant_op * arg1_constant = dynamic_cast<const constant_op *>(
		&arg1->node()->operation());
	const constant_op * arg2_constant = dynamic_cast<const constant_op *>(
		&arg2->node()->operation());

	if (arg1_constant && arg2_constant) {
		return jive_binop_reduction_constants;
	}

	const slice_op * arg1_slice = dynamic_cast<const slice_op *>(
		&arg1->node()->operation());
	const slice_op * arg2_slice = dynamic_cast<const slice_op *>(
		&arg2->node()->operation());

	if (arg1_slice && arg2_slice){
		jive::output * origin1 = arg1->node()->inputs[0]->origin();
		jive::output * origin2 = arg2->node()->inputs[0]->origin();

		if (origin1 == origin2 && arg1_slice->high() == arg2_slice->low()) {
			return jive_binop_reduction_merge;
		}

		/* FIXME: support sign bit */
	}

	return jive_binop_reduction_none;
}

jive::output *
concat_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const constant_op & arg1_constant = static_cast<const constant_op &>(
			arg1->node()->operation());
		const constant_op & arg2_constant = static_cast<const constant_op &>(
			arg2->node()->operation());

		size_t nbits = arg1_constant.value().size() + arg2_constant.value().size();
		char bits[nbits];
		memcpy(bits, &arg1_constant.value()[0], arg1_constant.value().size());
		memcpy(
			bits + arg1_constant.value().size(),
			&arg2_constant.value()[0],
			arg2_constant.value().size());

		return jive_bitconstant(graph, nbits, bits);
	}

	if (path == jive_binop_reduction_merge) {
		const slice_op * arg1_slice = static_cast<const slice_op *>(
			&arg1->node()->operation());
		const slice_op * arg2_slice = static_cast<const slice_op *>(
			&arg2->node()->operation());

		jive::output * origin1 = arg1->node()->inputs[0]->origin();

		return jive_bitslice(origin1, arg1_slice->low(), arg2_slice->high());

		/* FIXME: support sign bit */
	}

	return NULL;
}

jive_binary_operation_flags
concat_op::flags() const noexcept
{
	return jive_binary_operation_associative;
}

std::string
concat_op::debug_string() const
{
	return "BITCONCAT";
}

std::unique_ptr<jive::operation>
concat_op::copy() const
{
	return std::unique_ptr<jive::operation>(new concat_op(*this));
}

}
}
