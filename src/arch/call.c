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

jive_node *
call_operation::create_node(
	jive_region * region,
	size_t narguments_given,
	jive::output * const arguments[]) const
{
	jive_call_node * node = new jive_call_node(*this);
	node->class_ = &JIVE_CALL_NODE;

	const jive::base::type * argtypes[narguments()];
	for (size_t n = 0; n < narguments(); ++n) {
		argtypes[n] = &argument_type(n);
	}
	const jive::base::type * restypes[nresults()];
	for (size_t n = 0; n < nresults(); ++n) {
		restypes[n] = &result_type(n);
	}

	jive_node_init_(node, region,
		narguments(), argtypes, arguments,
		nresults(), restypes);

	return node;
}

std::string
call_operation::debug_string() const
{
	return "CALL";
}

}

const jive_node_class JIVE_CALL_NODE = {
	parent : &JIVE_NODE,
	name : "CALL",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

static inline jive_region *
call_node_region_innermost(jive::output * target_address, size_t narguments,
	jive::output * const arguments[])
{
	size_t i;
	jive::output * tmp[narguments+1];
	for(i = 0; i < narguments; i++){
		tmp[i] = arguments[i];
	}
	tmp[i] = target_address;
	
	return jive_region_innermost(narguments + 1, tmp);
}

jive_node *
jive_call_by_address_node_create(jive_region * region,
	jive::output * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	jive::output * call_args[narguments + 1];
	call_args[0] = target_address;

	std::vector<std::unique_ptr<jive::base::type>> argtypes;
	argtypes.emplace_back(new jive::addr::type());
	for (size_t n = 0; n < narguments; ++n) {
		argtypes.emplace_back(arguments[n]->type().copy());
		call_args[n + 1] = arguments[n];
	}

	std::vector<std::unique_ptr<jive::base::type>> restypes;
	for (size_t n = 0; n < nreturns; ++n) {
		restypes.emplace_back(return_types[n]->copy());
	}

	jive::call_operation op(
		calling_convention,
		std::move(argtypes),
		std::move(restypes));

	return op.create_node(
		region, narguments + 1, call_args);
}

jive::output * const *
jive_call_by_address_create(jive::output * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	jive_region * region = call_node_region_innermost(target_address, narguments, arguments);

	return &jive_call_by_address_node_create(region, target_address, calling_convention,
		narguments, arguments, nreturns, return_types)->outputs[0];
}

jive_node *
jive_call_by_bitstring_node_create(jive_region * region,
	jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	jive::output * call_args[narguments + 1];
	call_args[0] = target_address;

	std::vector<std::unique_ptr<jive::base::type>> argtypes;
	argtypes.emplace_back(new jive::bits::type(nbits));
	for (size_t n = 0; n < narguments; ++n) {
		argtypes.emplace_back(arguments[n]->type().copy());
		call_args[n + 1] = arguments[n];
	}

	std::vector<std::unique_ptr<jive::base::type>> restypes;
	for (size_t n = 0; n < nreturns; ++n) {
		restypes.emplace_back(return_types[n]->copy());
	}

	jive::call_operation op(
		calling_convention,
		std::move(argtypes),
		std::move(restypes));

	return op.create_node(
		region, narguments + 1, call_args);
}

jive::output * const *
jive_call_by_bitstring_create(jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[])
{
	jive_region * region = call_node_region_innermost(target_address, narguments, arguments);

	return &jive_call_by_bitstring_node_create(region, target_address, nbits, calling_convention,
		narguments, arguments, nreturns, return_types)->outputs[0];
}
