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
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma-normal-form.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/seqtype.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>
#include <jive/vsdg/traverser.h>

namespace jive {

gamma_tail_op::~gamma_tail_op() noexcept
{
}

size_t
gamma_tail_op::narguments() const noexcept
{
	return 1;
}

const base::type &
gamma_tail_op::argument_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
gamma_tail_op::debug_string() const
{
	return "GAMMA_TAIL";
}

std::unique_ptr<jive::operation>
gamma_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_tail_op(*this));
}

gamma_op::~gamma_op() noexcept
{
}

size_t
gamma_op::narguments() const noexcept
{
	return 1;
}

const base::type &
gamma_op::argument_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return predicate_type_;
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
	jive_graph * graph)
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


static jive::structural_node *
jive_gamma_create(
	jive::oport * predicate,
	const std::vector<const jive::base::type*> & types,
	const std::vector<std::vector<jive::oport*>> & alternatives)
{
	auto nvalues = types.size();
	auto nalternatives = alternatives.size();
	auto parent = predicate->region();
	auto graph = parent->graph();

	auto gamma = new jive::structural_node(jive::gamma_op(nalternatives), parent, nalternatives);
	gamma->add_input(&gamma->operation().argument_type(0), predicate);

	std::vector<std::vector<jive::argument*>> arguments;
	for (size_t i = 0; i < nalternatives; i++) {
		std::vector<jive::argument*> args;
		JIVE_DEBUG_ASSERT(alternatives[i].size() == nvalues);
		for (size_t n = 0; n < nvalues; n++) {
			auto arg_gate = graph->create_gate(*types[n], jive::detail::strfmt("arg_", gamma, "_", n*i));

			auto input = gamma->add_input(arg_gate, alternatives[i][n]);
			args.push_back(gamma->subregion(i)->add_argument(input, arg_gate));
		}
		arguments.push_back(args);
	}

	JIVE_DEBUG_ASSERT(arguments.size() == nalternatives);
	for (size_t n = 0; n < nvalues; n++) {
		auto res_gate = graph->create_gate(*types[n], jive::detail::strfmt("res_", gamma, "_", n));
		auto output = gamma->add_output(res_gate);
		for (size_t i = 0; i < nalternatives; i++) {
			JIVE_DEBUG_ASSERT(arguments[i].size() == nvalues);
			gamma->subregion(i)->add_result(arguments[i][n], output, res_gate);
		}
	}

	return gamma;
}

std::vector<jive::oport *>
jive_gamma(jive::oport * predicate,
	const std::vector<const jive::base::type*> & types,
	const std::vector<std::vector<jive::oport*>> & alternatives)
{
	/*
		FIXME: This code is supposed to be in gamma-normal-form.c and nowhere else. However,
		we would need to extend the gamma operator with the types vector in order to make
		this possible.
	*/

	if (auto ctl = dynamic_cast<const jive::ctl::type*>(&predicate->type())) {
		if (ctl->nalternatives() != alternatives.size())
			throw jive::compiler_error("Incorrect number of alternatives.");
	}

	for (size_t n = 0; n < alternatives.size(); n++) {
		if (alternatives[n].size() != types.size())
			throw jive::compiler_error("Incorrect number of values for an alternative.");
	}

	/*
		FIXME: This is currently not guarded by any normal form attribute. We could make
		it a special case where two regions are equal, but we have to introduce this
		optimization first.
	*/
	if (alternatives.size() == 1)
		return alternatives[0];

	jive_graph * graph = predicate->region()->graph();
	auto nf = static_cast<jive::gamma_normal_form*>(graph->node_normal_form(typeid(jive::gamma_op)));

	if (nf->get_mutable() && nf->get_predicate_reduction()) {
		if (auto op = dynamic_cast<const jive::ctl::constant_op*>(
				&dynamic_cast<jive::output*>(predicate)->node()->operation()))
		{
				return alternatives[op->value().alternative()];
		}
	}

	std::vector<jive::oport*> results;
	auto node = jive_gamma_create(predicate, types, alternatives);
	for (size_t n = 0; n < node->noutputs(); n++)
		results.push_back(node->output(n));

	/*
		FIXME: this algorithm is O(n^2), since we have to iterate through all inputs/outputs
		that have a greater index than the deleted one
	*/
	if (nf->get_mutable() && nf->get_invariant_reduction()) {
		results.clear();
		size_t nalternatives = alternatives.size();
		for (size_t v = 0; v < node->noutputs(); v++) {
				auto res0 = node->subregion(0)->result(v);
				auto arg0 = dynamic_cast<const jive::argument*>(res0->origin());
				if (!arg0)
					continue;

			size_t n;
			for (n = 1; n < nalternatives; n++) {
				auto arg = dynamic_cast<const jive::argument*>(node->subregion(n)->result(v)->origin());
				if (arg || arg->input()->origin() != arg0->input()->origin())
					break;
			}
			if (n == nalternatives) {
				results.push_back(arg0->input()->origin());

				/* FIXME: ugh, this should be done by DNE */
				node->remove_output(v);
				jive::argument * argument = nullptr;
				for (size_t n = 0; n < nalternatives; n++) {
					argument = static_cast<jive::argument*>(node->subregion(n)->result(v)->origin());
					node->subregion(n)->remove_result(v);
					node->subregion(n)->remove_argument(argument->index());
				}
				node->remove_input(argument->input()->index());
			} else
				results.push_back(node->output(v));
		}
	}

	return results;
}
