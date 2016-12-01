/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/call.h>

#include <jive/arch/addresstype.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/ptr-collection.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {

call_operation::~call_operation() noexcept {}

call_operation::call_operation(
	const jive_calling_convention * calling_convention,
	const std::vector<std::unique_ptr<jive::base::type>> & argument_types,
	const std::vector<std::unique_ptr<jive::base::type>> & result_types)
	: calling_convention_(calling_convention)
	, argument_types_(detail::unique_ptr_vector_copy(argument_types))
	, result_types_(detail::unique_ptr_vector_copy(result_types))
{
}

call_operation::call_operation(const call_operation & other)
	: call_operation(other.calling_convention_, other.argument_types_, other.result_types_)
{
}

bool
call_operation::operator==(const operation & other) const noexcept
{
	const call_operation * op =
		dynamic_cast<const call_operation *>(&other);
	return op &&
		op->calling_convention() == calling_convention() &&
		detail::ptr_container_equals(op->argument_types(), argument_types()) &&
		detail::ptr_container_equals(op->result_types(), result_types());
}

size_t
call_operation::narguments() const noexcept
{
	return argument_types_.size();
}

const jive::base::type &
call_operation::argument_type(size_t index) const noexcept
{
	return *argument_types_[index];
}

size_t
call_operation::nresults() const noexcept
{
	return result_types_.size();
}

const jive::base::type &
call_operation::result_type(size_t index) const noexcept
{
	return *result_types_[index];
}
std::string
call_operation::debug_string() const
{
	return "CALL";
}

std::unique_ptr<jive::operation>
call_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new call_operation(*this));
}

}

jive::node *
jive_call_by_address_node_create(jive::region * region,
	jive::oport * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, jive::oport * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	std::vector<jive::oport*> call_args;
	call_args.push_back(target_address);

	std::vector<std::unique_ptr<jive::base::type>> argtypes;
	argtypes.emplace_back(new jive::addr::type());
	for (size_t n = 0; n < narguments; ++n) {
		argtypes.emplace_back(arguments[n]->type().copy());
		call_args.push_back(arguments[n]);
	}

	std::vector<std::unique_ptr<jive::base::type>> restypes;
	for (size_t n = 0; n < nreturns; ++n) {
		restypes.emplace_back(return_types[n]->copy());
	}

	jive::call_operation op(
		calling_convention,
		std::move(argtypes),
		std::move(restypes));

	return jive_opnode_create(op, region, call_args);
}

std::vector<jive::oport*>
jive_call_by_address_create(jive::oport * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::oport * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	jive::region * region = target_address->region();
	jive::node * node = jive_call_by_address_node_create(region, target_address, calling_convention,
		narguments, arguments, nreturns, return_types);

	std::vector<jive::oport*> results;
	for (size_t n = 0; n < node->noutputs(); n++)
		results.push_back(node->output(n));

	return results;
}

jive::node *
jive_call_by_bitstring_node_create(jive::region * region,
	jive::oport * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::oport * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	std::vector<jive::oport*> call_args;
	call_args.push_back(target_address);

	std::vector<std::unique_ptr<jive::base::type>> argtypes;
	argtypes.emplace_back(new jive::bits::type(nbits));
	for (size_t n = 0; n < narguments; ++n) {
		argtypes.emplace_back(arguments[n]->type().copy());
		call_args.push_back(arguments[n]);
	}

	std::vector<std::unique_ptr<jive::base::type>> restypes;
	for (size_t n = 0; n < nreturns; ++n) {
		restypes.emplace_back(return_types[n]->copy());
	}

	jive::call_operation op(
		calling_convention,
		std::move(argtypes),
		std::move(restypes));

	return jive_opnode_create(op, region, call_args);
}

std::vector<jive::oport*>
jive_call_by_bitstring_create(jive::oport * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::oport * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	jive::region * region = target_address->region();
	jive::node * node = jive_call_by_bitstring_node_create(region, target_address, nbits,
		calling_convention, narguments, arguments, nreturns, return_types);

	std::vector<jive::oport*> results;
	for (size_t n = 0; n < node->noutputs(); n++)
		results.push_back(node->output(n));

	return results;
}
