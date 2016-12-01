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
#include <jive/vsdg/substitution.h>

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

static jive::node *
jive_lambda_node_create(jive::region * function_region)
{
	size_t ndependencies = function_region->top()->ninputs();
	size_t narguments = function_region->top()->noutputs() - ndependencies - 1;
	size_t nreturns = function_region->bottom()->ninputs()-1;

	const jive::base::type * argument_types[narguments];
	std::vector<std::string> argument_names;
	for (size_t n = 0; n < narguments; n++) {
		argument_types[n] = &function_region->top()->output(n+1)->type();
		argument_names.push_back(function_region->top()->output(n+1)->gate()->name());
	}
	const jive::base::type * return_types[nreturns];
	std::vector<std::string> result_names;
	for (size_t n = 0; n < nreturns; n++) {
		return_types[n] = &function_region->bottom()->input(n+1)->type();
		result_names.push_back(function_region->bottom()->input(n+1)->gate()->name());
	}
	
	jive::fct::type function_type(
		narguments, argument_types,
		nreturns, return_types);
	
	jive::fct::lambda_op op(
		std::move(function_type),
		std::move(argument_names),
		std::move(result_names));

	jive::output * tmp = function_region->bottom()->output(0);
	return jive_opnode_create(op, function_region->parent(), {tmp});
}


bool
jive_lambda_is_self_recursive(const jive::node * self)
{
	JIVE_DEBUG_ASSERT(self->noutputs() == 1);

	auto lambda_region = self->input(0)->origin()->region();
	JIVE_DEBUG_ASSERT(self->region() != lambda_region);

	if (jive_phi_region_const_cast(self->region()) == NULL)
		return false;

	/* find index of lambda output in the phi leave node */
	size_t index = self->region()->top()->noutputs();
	for (auto user : self->output(0)->users) {
		auto input = dynamic_cast<jive::input*>(user);
		if (dynamic_cast<const jive::phi_tail_op *>(&input->node()->operation())) {
			index = user->index();
			break;
		}
	}
	JIVE_DEBUG_ASSERT(index != self->region()->top()->noutputs());

	/* the lambda is self-recursive if one of its external dependencies originates from the same
	*  index in the phi enter node
	*/
	for (size_t n = 0; n < lambda_region->top()->ninputs(); n++) {
		jive::input * input = lambda_region->top()->input(n);
		if (input->origin()->index() == index)
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
	jive::node * enter = self->region->top();
	jive_graph * graph = self->region->graph();

	jive::fct::lambda_dep depvar;
	jive::gate * gate = graph->create_gate(
		value->type(),
		jive::detail::strfmt("dep_", enter, "_", self->depvars.size()));
	depvar.input = enter->add_input(gate, value);
	depvar.output = enter->add_output(gate);
	self->depvars.push_back(depvar);

	return depvar;
}

}
}

jive_lambda *
jive_lambda_begin(
	jive::region * parent,
	size_t narguments,
	const jive::base::type * const argument_types[],
	const char * const argument_names[])
{
	jive_graph * graph = parent->graph();

	jive_lambda * lambda = new jive_lambda;
	lambda->region = new jive::region(parent, parent->graph());
	lambda->arguments = new jive::oport*[narguments];
	lambda->narguments = narguments;

	jive_opnode_create(jive::fct::lambda_head_op(), lambda->region, {});

	size_t n;
	for (n = 0; n < narguments; n++) {
		jive::gate * gate = graph->create_gate(*argument_types[n], argument_names[n]);
		lambda->arguments[n] = lambda->region->top()->add_output(gate);
	}

	return lambda;
}

jive::oport *
jive_lambda_end(jive_lambda * self,
	size_t nresults, const jive::base::type * const result_types[], jive::oport * const results[])
{
	jive::region * region = self->region;
	jive_graph * graph = region->graph();

	jive::output * tmp = region->top()->output(0);
	jive::node * leave = jive_opnode_create(jive::fct::lambda_tail_op(), region, {tmp});

	size_t n;
	for (n = 0; n < nresults; n++) {
		char gate_name[80];
		snprintf(gate_name, sizeof(gate_name), "res_%p_%zd", leave, n);
		jive::gate * gate = graph->create_gate(*result_types[n], gate_name);
		leave->add_input(gate, results[n]);
	}

	jive::node * anchor = jive_lambda_node_create(region);
	JIVE_DEBUG_ASSERT(anchor->noutputs() == 1);

	delete[] self->arguments;
	delete self;

	return anchor->output(0);
}

/* lambda inlining */

void
jive_inline_lambda_apply(jive::node * apply_node)
{
	jive::node * lambda_node = dynamic_cast<jive::output*>(apply_node->input(0)->origin())->node();
	
	const jive::fct::lambda_op & op = dynamic_cast<const jive::fct::lambda_op &>(
		lambda_node->operation());
	
	jive::region * function_region = lambda_node->input(0)->origin()->region();
	jive::node * head = function_region->top();
	jive::node * tail = function_region->bottom();
	
	jive::substitution_map substitution;
	for(size_t n = 0; n < op.function_type().narguments(); n++) {
		jive::output * output = jive_node_get_gate_output(head, op.argument_names()[n].c_str());
		substitution.insert(output, dynamic_cast<jive::output*>(apply_node->input(n+1)->origin()));
	}
	
	function_region->copy(apply_node->region(), substitution, false, false);
	
	for(size_t n = 0; n < op.function_type().nreturns(); n++) {
		jive::input * input = jive_node_get_gate_input(tail, op.result_names()[n].c_str());
		jive::oport * substituted = substitution.lookup(input->origin());
		jive::output * output = apply_node->output(n);
		output->replace(substituted);
	}
}
