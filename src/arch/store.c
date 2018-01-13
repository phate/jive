/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/store.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/rvsdg/graph.h>
#include <jive/types/bitstring/type.h>

static std::vector<jive::output*>
jive_store_node_normalized_create(
	const jive::node_normal_form * nf,
	jive::graph * graph,
	const jive::simple_op & op,
	jive::output * address,
	jive::output * value,
	size_t nstates, jive::output * const states[])
{
	std::vector<jive::output*> args = {address, value};
	for (size_t n = 0; n < nstates; ++n) {
		args.push_back(states[n]);
	}

	return jive::create_normalized(address->region(), op, args);
}


namespace jive {

store_op::~store_op() noexcept
{
}

bool
store_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const store_op *>(&other);
	return op
	    && op->address_ == address_
	    && op->value_ == value_
	    && op->states_ == states_;
}

size_t
store_op::narguments() const noexcept
{
	return 2 + states_.size();
}

const jive::port &
store_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());

	if (index == 0)
		return address_;

	if (index == 1)
		return value_;

	return states_[index-2];
}

size_t
store_op::nresults() const noexcept
{
	return states_.size();
}

const jive::port &
store_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return states_[index];
}

std::string
store_op::debug_string() const
{
	return "STORE";
}

std::unique_ptr<jive::operation>
store_op::copy() const
{
	return std::unique_ptr<jive::operation>(new store_op(*this));
}

}

/* store_node */

std::vector<jive::output*>
jive_store_by_address_create(jive::output* address,
	const jive::valuetype * datatype, jive::output * value,
	size_t nstates, jive::output * const istates[])
{
	jive::graph * graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::store_op));
	
	std::vector<std::unique_ptr<jive::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::statetype &>(istates[n]->type()).copy());
	}

	jive::store_op op(jive::addrtype(), state_types, *datatype);

	return jive_store_node_normalized_create(nf, graph, op, address, value, nstates, istates);
}

std::vector<jive::output*>
jive_store_by_bitstring_create(jive::output * address, size_t nbits,
	const jive::valuetype * datatype, jive::output * value,
	size_t nstates, jive::output * const istates[])
{
	jive::graph * graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::store_op));

	std::vector<std::unique_ptr<jive::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::statetype &>(istates[n]->type()).copy());
	}

	jive::store_op op(jive::bits::type(nbits), state_types, *datatype);

	return jive_store_node_normalized_create(nf, graph, op, address, value, nstates, istates);
}
