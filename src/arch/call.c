/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/ptr-collection.h>

namespace jive {

call_operation::~call_operation() noexcept {}

call_operation::call_operation(
	const jive_calling_convention * calling_convention,
	const std::vector<std::unique_ptr<jive::type>> & argument_types,
	const std::vector<std::unique_ptr<jive::type>> & result_types)
: calling_convention_(calling_convention)
{
	for (const auto & type : argument_types)
		arguments_.push_back({std::move(type->copy())});
	for (const auto & type : result_types)
		results_.push_back({std::move(type->copy())});
}

bool
call_operation::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const call_operation *>(&other);
	return op
	    && op->calling_convention() == calling_convention()
	    && op->arguments_ == arguments_
	    && op->results_ == results_;
}

size_t
call_operation::narguments() const noexcept
{
	return arguments_.size();
}

const jive::port &
call_operation::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index];
}

size_t
call_operation::nresults() const noexcept
{
	return results_.size();
}

const jive::port &
call_operation::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index];
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
	jive::output * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::type * const return_types[])
{
	std::vector<jive::output*> call_args;
	call_args.push_back(target_address);

	std::vector<std::unique_ptr<jive::type>> argtypes;
	argtypes.emplace_back(new jive::addrtype());
	for (size_t n = 0; n < narguments; ++n) {
		argtypes.emplace_back(arguments[n]->type().copy());
		call_args.push_back(arguments[n]);
	}

	std::vector<std::unique_ptr<jive::type>> restypes;
	for (size_t n = 0; n < nreturns; ++n) {
		restypes.emplace_back(return_types[n]->copy());
	}

	jive::call_operation op(
		calling_convention,
		std::move(argtypes),
		std::move(restypes));

	return region->add_simple_node(op, call_args);
}

std::vector<jive::output*>
jive_call_by_address_create(jive::output * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::type * const return_types[])
{
	jive::region * region = target_address->region();
	jive::node * node = jive_call_by_address_node_create(region, target_address, calling_convention,
		narguments, arguments, nreturns, return_types);

	std::vector<jive::output*> results;
	for (size_t n = 0; n < node->noutputs(); n++)
		results.push_back(node->output(n));

	return results;
}

jive::node *
jive_call_by_bitstring_node_create(jive::region * region,
	jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::type * const return_types[])
{
	std::vector<jive::output*> call_args;
	call_args.push_back(target_address);

	std::vector<std::unique_ptr<jive::type>> argtypes;
	argtypes.emplace_back(new jive::bits::type(nbits));
	for (size_t n = 0; n < narguments; ++n) {
		argtypes.emplace_back(arguments[n]->type().copy());
		call_args.push_back(arguments[n]);
	}

	std::vector<std::unique_ptr<jive::type>> restypes;
	for (size_t n = 0; n < nreturns; ++n) {
		restypes.emplace_back(return_types[n]->copy());
	}

	jive::call_operation op(
		calling_convention,
		std::move(argtypes),
		std::move(restypes));

	return region->add_simple_node(op, call_args);
}

std::vector<jive::output*>
jive_call_by_bitstring_create(jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::type * const return_types[])
{
	jive::region * region = target_address->region();
	jive::node * node = jive_call_by_bitstring_node_create(region, target_address, nbits,
		calling_convention, narguments, arguments, nreturns, return_types);

	std::vector<jive::output*> results;
	for (size_t n = 0; n < node->noutputs(); n++)
		results.push_back(node->output(n));

	return results;
}
