/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>

#include <stdio.h>
#include <string.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/phi.h>
#include <jive/vsdg/seqtype.h>
#include <jive/vsdg/structural_node.h>

/* lambda enter node */

/* lambda leave node */


/* lambda node */

namespace jive {
namespace fct {

lambda_head_op::~lambda_head_op() noexcept
{
}

size_t
lambda_head_op::nresults() const noexcept
{
	return 1;
}

const base::type &
lambda_head_op::result_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
lambda_head_op::debug_string() const
{
	return "LAMBDA_HEAD";
}

std::unique_ptr<jive::operation>
lambda_head_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lambda_head_op(*this));
}

lambda_tail_op::~lambda_tail_op() noexcept
{
}

size_t
lambda_tail_op::narguments() const noexcept
{
	return 1;
}

const base::type &
lambda_tail_op::argument_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
lambda_tail_op::debug_string() const
{
	return "LAMBDA_TAIL";
}

std::unique_ptr<jive::operation>
lambda_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lambda_tail_op(*this));
}

lambda_op::~lambda_op() noexcept
{
}

bool
lambda_op::operator==(const operation & other) const noexcept
{
	const lambda_op * op =
		dynamic_cast<const lambda_op *>(&other);
	return
		op &&
		op->function_type() == function_type() &&
		op->argument_names() == argument_names() &&
		op->result_names() == result_names();
}

size_t
lambda_op::narguments() const noexcept
{
	return 0;
}

size_t
lambda_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
lambda_op::result_type(size_t index) const noexcept
{
	return function_type();
}

std::string
lambda_op::debug_string() const
{
	return "LAMBDA";
}

std::unique_ptr<jive::operation>
lambda_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lambda_op(*this));
}

}
}

bool
jive_lambda_is_self_recursive(const jive::node * self)
{
	JIVE_DEBUG_ASSERT(self->noutputs() == 1);

	auto lambda = dynamic_cast<const jive::structural_node*>(self);

	auto phi_region = self->region();
	auto phi = phi_region->node();
	if (phi && typeid(phi->operation()) != typeid(jive::phi_op))
		return false;

	/* find index of lambda output in the phi leave node */
	size_t index = phi_region->nresults();
	for (const auto & user : self->output(0)->users) {
		if (dynamic_cast<const jive::result*>(user)) {
			index = user->index();
			break;
		}
	}
	if (index == phi_region->nresults())
		return false;

	/* the lambda is self-recursive if one of its external dependencies originates from the same
	*  index in the phi enter node
	*/
	for (size_t n = 0; n < lambda->ninputs(); n++) {
		if (lambda->input(n)->origin()->index() == index)
			return true;
	}

	return false;
}

/* lambda instantiation */

namespace jive {
namespace fct {

lambda_dep
lambda_dep_add(jive_lambda * self, jive::oport * value)
{
	auto graph = self->node->graph();

	jive::fct::lambda_dep depvar;
	auto gate = graph->create_gate(value->type(),
		jive::detail::strfmt("dep_", self->node, "_", self->depvars.size()));
	depvar.input = self->node->add_input(gate, value);
	depvar.output = self->node->subregion(0)->add_argument(depvar.input, gate);
	self->depvars.push_back(depvar);

	return depvar;
}

}
}

jive_lambda *
jive_lambda_begin(
	jive::region * parent,
	const std::vector<std::pair<const jive::base::type*, std::string>> & arguments,
	const std::vector<std::pair<const jive::base::type*, std::string>> & results)
{
	std::vector<std::string> argument_names;
	std::vector<const jive::base::type*> argument_types;
	for (const auto & arg : arguments) {
		argument_types.push_back(arg.first);
		argument_names.push_back(arg.second);
	}

	std::vector<std::string> result_names;
	std::vector<const jive::base::type*> result_types;
	for (const auto & result : results) {
		result_types.push_back(result.first);
		result_names.push_back(result.second);
	}

	jive::fct::type fcttype(argument_types.size(), &argument_types[0],
		result_types.size(), &result_types[0]);

	auto lambda = new jive_lambda;
	lambda->node = new jive::structural_node(
		jive::fct::lambda_op(fcttype, argument_names, result_names), parent, 1);
	lambda->region = lambda->node->subregion(0);
	lambda->arguments = new jive::oport*[arguments.size()];
	lambda->narguments = arguments.size();

	for (size_t n = 0; n < arguments.size(); n++) {
		auto gate = parent->graph()->create_gate(*arguments[n].first, arguments[n].second);
		lambda->arguments[n] = lambda->node->subregion(0)->add_argument(nullptr, gate);
	}

	return lambda;
}

jive::oport *
jive_lambda_end(jive_lambda * self,
	size_t nresults, const jive::base::type * const result_types[], jive::oport * const results[])
{
	auto node = self->node;
	auto graph = self->node->graph();
	auto region = node->subregion(0);
	auto op = static_cast<const jive::fct::lambda_op*>(&node->operation());
	auto fcttype = static_cast<const jive::fct::type*>(&op->result_type(0));

	if (nresults != fcttype->nreturns())
		throw std::logic_error("Incorrect number of results.");

	for (size_t n = 0; n < nresults; n++) {
		if (*result_types[n] != *fcttype->return_type(n))
			throw std::logic_error("Incorrect result type.");

		auto gate = graph->create_gate(*result_types[n], op->result_names()[n]);
		region->add_result(results[n], nullptr, gate);
	}

	node->add_output(fcttype);

	delete[] self->arguments;
	delete self;

	return node->output(0);
}
