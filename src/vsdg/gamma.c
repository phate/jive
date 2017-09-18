/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma.h>

#include <algorithm>
#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/util/strfmt.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

namespace jive {

/* gamma normal form */

static bool
is_predicate_reducible(const jive::structural_node * node)
{
	auto constant = node->input(0)->origin()->node();
	return constant && is_ctlconstant_op(constant->operation());
}

static void
perform_predicate_reduction(jive::structural_node * node)
{
	jive::gamma gamma(node);
	auto constant = gamma.predicate()->origin()->node();
	auto cop = static_cast<const jive::ctl::constant_op*>(&constant->operation());
	auto alternative = cop->value().alternative();

	jive::substitution_map smap;
	for (auto it = gamma.begin_entryvar(); it != gamma.end_entryvar(); it++)
		smap.insert(it->argument(alternative), it->input()->origin());

	gamma.subregion(alternative)->copy(gamma.region(), smap, false, false);

	for (auto it = gamma.begin_exitvar(); it != gamma.end_exitvar(); it++)
		it->output()->replace(smap.lookup(it->result(alternative)->origin()));

	remove(node);
}

static bool
perform_invariant_reduction(jive::structural_node * node)
{
	jive::gamma gamma(node);

	bool was_normalized = true;
	for (auto it = gamma.begin_exitvar(); it != gamma.end_exitvar(); it++) {
		auto argument = dynamic_cast<const jive::argument*>(it->result(0)->origin());
		if (!argument) continue;

		size_t n;
		auto input = argument->input();
		for (n = 1; n < it->nresults(); n++) {
			auto argument = dynamic_cast<const jive::argument*>(it->result(n)->origin());
			if (!argument && argument->input() != input)
				break;
		}

		if (n == it->nresults()) {
			it->output()->replace(argument->input()->origin());
			was_normalized = false;
		}
	}

	return was_normalized;
}

gamma_normal_form::~gamma_normal_form() noexcept
{}

gamma_normal_form::gamma_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph) noexcept
: structural_normal_form(operator_class, parent, graph)
, enable_predicate_reduction_(true)
, enable_invariant_reduction_(true)
{
	if (auto p = dynamic_cast<gamma_normal_form *>(parent)) {
		enable_predicate_reduction_ = p->enable_predicate_reduction_;
		enable_invariant_reduction_ = p->enable_invariant_reduction_;
	}
}

bool
gamma_normal_form::normalize_node(jive::node * node_) const
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::gamma_op*>(&node_->operation()));
	auto node = static_cast<jive::structural_node*>(node_);

	if (!get_mutable())
		return true;

	if (get_predicate_reduction() && is_predicate_reducible(node)) {
		perform_predicate_reduction(node);
		return false;
	}

	if (get_invariant_reduction())
		return perform_invariant_reduction(node);

	return true;
}

void
gamma_normal_form::set_predicate_reduction(bool enable)
{
	if (enable_predicate_reduction_ == enable) {
		return;
	}

	children_set<gamma_normal_form, &gamma_normal_form::set_predicate_reduction>(enable);

	enable_predicate_reduction_ = enable;

	if (enable && get_mutable())
		graph()->mark_denormalized();
}

void
gamma_normal_form::set_invariant_reduction(bool enable)
{
	if (enable_invariant_reduction_ == enable) {
		return;
	}

	children_set<gamma_normal_form, &gamma_normal_form::set_invariant_reduction>(enable);

	enable_invariant_reduction_ = enable;

	if (enable && get_mutable())
		graph()->mark_denormalized();
}

/* gamma operation */

gamma_op::~gamma_op() noexcept
{
}

std::string
gamma_op::debug_string() const
{
	return "GAMMA";
}

std::unique_ptr<jive::operation>
gamma_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_op(*this));
}

bool
gamma_op::operator==(const operation & other) const noexcept
{
	const gamma_op * op = dynamic_cast<const gamma_op*>(&other);
	return op && op->predicate_type_ == predicate_type_;
}

}

jive::node_normal_form *
jive_gamma_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	jive::gamma_normal_form * normal_form = new jive::gamma_normal_form(
		operator_class, parent, graph);
	return normal_form;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::gamma_op), jive_gamma_node_get_default_normal_form_);
}
