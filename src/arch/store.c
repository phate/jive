/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/store.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/memorytype.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdgroup.h>
#include <jive/types/union/unntype.h>
#include <jive/types/union/unnunify.h>
#include <jive/vsdg/graph.h>

static std::vector<jive::oport*>
jive_store_node_normalized_create(
	const jive::node_normal_form * nf,
	jive::graph * graph,
	const jive::operation & op,
	jive::oport * address,
	jive::oport * value,
	size_t nstates, jive::oport * const states[])
{
	std::vector<jive::oport*> args = {address, value};
	for (size_t n = 0; n < nstates; ++n) {
		args.push_back(states[n]);
	}

	auto tmp = nf->normalized_create(address->region(), op, args);
	return {tmp.begin(), tmp.end()};
}


namespace jive {

store_op::~store_op() noexcept
{
}

bool
store_op::operator==(const operation & other) const noexcept
{
	const store_op * op =
		dynamic_cast<const store_op *>(&other);
	return (
		op &&
		op->address_type() == address_type() &&
		op->data_type() == data_type() &&
		detail::ptr_container_equals(op->state_types(), state_types())
	);
}

size_t
store_op::narguments() const noexcept
{
	return 2 + state_types().size();
}

const jive::base::type &
store_op::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return address_type();
	} else if (index == 1) {
		return data_type();
	} else {
		return *state_types()[index - 2];
	}
}

size_t
store_op::nresults() const noexcept
{
	return state_types().size();
}

const jive::base::type &
store_op::result_type(size_t index) const noexcept
{
	return *state_types()[index];
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

std::vector<jive::oport*>
jive_store_by_address_create(jive::oport* address,
	const jive::value::type * datatype, jive::oport * value,
	size_t nstates, jive::oport * const istates[])
{
	jive::graph * graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::store_op));
	
	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(istates[n]->type()).copy());
	}

	jive::store_op op(jive::addr::type(), state_types, *datatype);

	return jive_store_node_normalized_create(nf, graph, op, address, value, nstates, istates);
}

std::vector<jive::oport*>
jive_store_by_bitstring_create(jive::oport * address, size_t nbits,
	const jive::value::type * datatype, jive::oport * value,
	size_t nstates, jive::oport * const istates[])
{
	jive::graph * graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::store_op));

	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(istates[n]->type()).copy());
	}

	jive::store_op op(jive::bits::type(nbits), state_types, *datatype);

	return jive_store_node_normalized_create(nf, graph, op, address, value, nstates, istates);
}
