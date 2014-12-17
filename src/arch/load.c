/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load-normal-form.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype.h>

static jive::output *
jive_load_node_normalized_create(
	const jive::node_normal_form * nf,
	jive_graph * graph,
	const jive::operation & op,
	jive::output * address,
	size_t nstates, jive::output * const states[])
{
	std::vector<jive::output *> args = {address};
	for (size_t n = 0; n < nstates; ++n) {
		args.push_back(states[n]);
	}
	return nf->normalized_create(op, args)[0];
}

namespace jive {

load_op::~load_op() noexcept
{
}

bool
load_op::operator==(const operation & other) const noexcept
{
	const load_op * op =
		dynamic_cast<const load_op *>(&other);
	return (
		op &&
		op->address_type() == address_type() &&
		op->data_type() == data_type() &&
		detail::ptr_container_equals(op->state_types(), state_types())
	);
}

size_t
load_op::narguments() const noexcept
{
	return 1 + state_types().size();
}

const jive::base::type &
load_op::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return address_type();
	} else {
		return *state_types()[index - 1];
	}
}

size_t
load_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
load_op::result_type(size_t index) const noexcept
{
	return data_type();
}

jive_node *
load_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

std::string
load_op::debug_string() const
{
	return "LOAD";
}

std::unique_ptr<jive::operation>
load_op::copy() const
{
	return std::unique_ptr<jive::operation>(new load_op(*this));
}

}

jive::node_normal_form *
jive_load_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive::node_normal_form * nf = new jive::load_normal_form(
		operator_class, parent, graph);

	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::load_op), jive_load_get_default_normal_form_);
}

namespace {

template<typename T>
class ptr_array {
public:
	typedef const T value_type;
	typedef const T * iterator;

	inline ptr_array(const T * begin, const T * end) noexcept
		: begin_(begin), end_(end)
	{
	}

	inline iterator begin() const { return begin_; }
	inline iterator end() const { return end_; }

private:
	const T * begin_;
	const T * end_;
};

template<typename T>
inline ptr_array<T>
make_ptr_array(const T * begin, const T * end)
{
	return ptr_array<T>(begin, end);
}

}

jive::output *
jive_load_by_address_create(jive::output * address,
	const jive::value::type * datatype,
	size_t nstates, jive::output * const states[])
{
	jive_graph * graph = address->node()->region->graph;
	const jive::node_normal_form * nf = jive_graph_get_nodeclass_form(
		address->node()->region->graph, typeid(jive::load_op));
	
	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::addr::type(), state_types, *datatype);

	return jive_load_node_normalized_create(nf, graph, op, address, nstates, states);
}

jive::output *
jive_load_by_bitstring_create(jive::output * address, size_t nbits,
	const jive::value::type * datatype,
	size_t nstates, jive::output * const states[])
{
	jive_graph * graph = address->node()->region->graph;
	const jive::node_normal_form * nf = jive_graph_get_nodeclass_form(
		address->node()->region->graph, typeid(jive::load_op));
	
	std::vector<std::unique_ptr<jive::state::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::state::type &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::bits::type(nbits), state_types, *datatype);

	return jive_load_node_normalized_create(nf, graph, op, address, nstates, states);
}
