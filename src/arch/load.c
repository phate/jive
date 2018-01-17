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

namespace {

jive::node_normal_form *
jive_load_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	return new jive::load_normal_form(operator_class, parent, graph);
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::load_op), jive_load_get_default_normal_form_);
}

}

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

	return l_op.addresstype() == s_op->addresstype() && l_op.valuetype() == s_op->valuetype();
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
			node->output(0)->divert_users(store_node->input(1)->origin());
			remove(node);
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

/* load operator */

load_op::~load_op() noexcept
{}

bool
load_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const load_op *>(&other);
	return op
	    && op->addresstype() == addresstype()
	    && op->valuetype() == valuetype()
	    && op->narguments() == narguments();
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

std::vector<jive::port>
load_op::create_operands(const jive::valuetype & address, size_t nstates)
{
	std::vector<jive::port> operands({address});
	for (size_t n = 0; n < nstates; n++)
		operands.push_back({memtype()});

	return operands;
}

/* address load operator */

addrload_op::~addrload_op()
{}

/* bitstring load operator */

bitload_op::~bitload_op()
{}

}
