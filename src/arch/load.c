/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/load.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/store.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/types/bitstring/type.h>

namespace jive {

/* load normal form */

load_normal_form::~load_normal_form() noexcept
{
}

load_normal_form::load_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph) noexcept
	: simple_normal_form(operator_class, parent, graph)
	, enable_reducible_(true)
{
	if (auto p = dynamic_cast<load_normal_form *>(parent)) {
		enable_reducible_ = p->enable_reducible_;
	}
}

static bool
is_matching_store_op(const jive::load_op & l_op, const jive::operation & op)
{
	const jive::store_op * s_op = dynamic_cast<const jive::store_op *>(&op);
	if (!s_op)
		return false;

	return l_op.address_type() == s_op->address_type() && l_op.data_type() == s_op->data_type();
}

static bool is_matching_store_node(
	const jive::load_op & l_op, const jive::output * address,
	const jive::node * node) {
	return
		is_matching_store_op(l_op, node->operation()) &&
		node->input(0)->origin() == address;
}

bool
load_normal_form::normalize_node(jive::node * node) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(node->operation());
		auto address = node->input(0)->origin();
		jive::node * store_node =
			(node->ninputs() >= 2 &&
				is_matching_store_node(l_op, address,
				node->input(1)->origin()->node())) ?
			node->input(1)->origin()->node() : nullptr;
		for (size_t n = 2; n < node->ninputs(); ++n) {
			if (node->input(n)->origin()->node() != store_node) {
				store_node = nullptr;
			}
		}
		if (store_node) {
			node->output(0)->replace(store_node->input(1)->origin());
			node->region()->remove_node(node);
			return false;
		}
	}

	return simple_normal_form::normalize_node(node);
}

std::vector<jive::output*>
load_normal_form::normalized_create(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & args) const
{
	if (get_mutable() && get_reducible()) {
		const jive::load_op & l_op = static_cast<const jive::load_op &>(op);
		auto addr = args[0];
		jive::node * store_node = nullptr;
		if (args.size() >= 2 && args[1]->node() && is_matching_store_node(l_op, addr, args[1]->node()))
			store_node = args[1]->node();

		for (size_t n = 2; n < args.size(); ++n) {
			if (args[n]->node() != store_node) {
				store_node = nullptr;
				break;
			}
		}
		if (store_node) {
			return {store_node->input(1)->origin()};
		}
	}

	return simple_normal_form::normalized_create(region, op, args);
}

void
load_normal_form::set_reducible(bool enable)
{
	if (get_reducible() == enable) {
		return;
	}

	children_set<load_normal_form, &load_normal_form::set_reducible>(enable);

	enable_reducible_ = enable;
	if (get_mutable() && enable)
		graph()->mark_denormalized();
}

}

static jive::output *
jive_load_node_normalized_create(
	const jive::node_normal_form * nf,
	jive::graph * graph,
	const jive::simple_op & op,
	jive::output * address,
	size_t nstates, jive::output * const states[])
{
	std::vector<jive::output*> args = {address};
	for (size_t n = 0; n < nstates; ++n) {
		args.push_back(states[n]);
	}

	return jive::create_normalized(address->region(), op, args)[0];
}

namespace jive {

/* load operator */

load_op::~load_op() noexcept
{
}

bool
load_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const load_op *>(&other);
	return op
	    && op->value_ == value_
	    && op->address_ == address_
			&& op->states_ == states_;
}

size_t
load_op::narguments() const noexcept
{
	return 1 + states_.size();
}

const jive::port &
load_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());

	if (index == 0)
		return address_;

	return states_[index-1];
}

size_t
load_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
load_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return value_;
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
	jive::graph * graph)
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

jive::output *
jive_load_by_address_create(jive::output * address,
	const jive::valuetype * datatype,
	size_t nstates, jive::output * const states[])
{
	auto graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::load_op));
	
	std::vector<std::unique_ptr<jive::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::statetype &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::addrtype(), state_types, *datatype);

	return jive_load_node_normalized_create(nf, graph, op, address, nstates, states);
}

jive::output *
jive_load_by_bitstring_create(jive::output * address, size_t nbits,
	const jive::valuetype * datatype,
	size_t nstates, jive::output * const states[])
{
	auto graph = address->region()->graph();
	const auto nf = graph->node_normal_form(typeid(jive::load_op));
	
	std::vector<std::unique_ptr<jive::type>> state_types;
	for (size_t n = 0; n < nstates; ++n) {
		state_types.emplace_back(
			dynamic_cast<const jive::statetype &>(states[n]->type()).copy());
	}

	jive::load_op op(jive::bits::type(nbits), state_types, *datatype);

	return jive_load_node_normalized_create(nf, graph, op, address, nstates, states);
}
