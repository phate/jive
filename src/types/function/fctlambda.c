/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>

#include <stdio.h>
#include <string.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
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

static jive_node *
jive_lambda_node_create(jive_region * function_region)
{
	size_t narguments = function_region->top->noutputs - 1;
	size_t nreturns = function_region->bottom->ninputs - 1;
	
	const jive::base::type * argument_types[narguments];
	std::vector<std::string> argument_names;
	for (size_t n = 0; n < narguments; n++) {
		argument_types[n] = &function_region->top->outputs[n+1]->type();
		argument_names.push_back(function_region->top->outputs[n+1]->gate->name);
	}
	const jive::base::type * return_types[nreturns];
	std::vector<std::string> result_names;
	for (size_t n = 0; n < nreturns; n++) {
		return_types[n] = &function_region->bottom->inputs[n+1]->type();
		result_names.push_back(function_region->bottom->inputs[n+1]->gate->name);
	}
	
	jive::fct::type function_type(
		narguments, argument_types,
		nreturns, return_types);
	
	jive::fct::lambda_op op(
		std::move(function_type),
		std::move(argument_names),
		std::move(result_names));

	return op.create_node(function_region->parent, 1, &function_region->bottom->outputs[0]);
}


bool
jive_lambda_is_self_recursive(const jive_node * self)
{
	JIVE_DEBUG_ASSERT(self->noutputs == 1);

	const jive_region * lambda_region = jive_node_anchored_region(self, 0);

	if (jive_phi_region_const_cast(self->region) == NULL)
		return false;

	/* find index of lambda output in the phi leave node */
	jive::input * user;
	size_t index = self->region->top->noutputs;
	JIVE_LIST_ITERATE(self->outputs[0]->users, user, output_users_list) {
		if (dynamic_cast<const jive::phi_tail_op *>(&user->node()->operation())) {
			index = user->index();
			break;
		}
	}
	JIVE_DEBUG_ASSERT(index != self->region->top->noutputs);

	/* the lambda is self recursive if the function input of an apply node in the lambda region
		originates from the same index in the phi enter node */
	jive_region_hull_entry * entry;
	JIVE_LIST_ITERATE(lambda_region->hull, entry, input_hull_list) {
		if (!dynamic_cast<const jive::fct::apply_op *>(&entry->input->node()->operation()))
			continue;
		if (!dynamic_cast<const jive::phi_head_op *>(&entry->input->producer()->operation()))
			continue;

		if (entry->input->origin()->index == index)
			return true;
	}

	return false;
}

/* lambda instantiation */

jive_lambda *
jive_lambda_begin(
	jive_region * parent,
	size_t narguments,
	const jive::base::type * const argument_types[],
	const char * const argument_names[])
{
	struct jive_graph * graph = parent->graph;

	jive_lambda * lambda = new jive_lambda;
	lambda->region = jive_region_create_subregion(parent);
	lambda->arguments = new jive::output*[narguments];
	lambda->narguments = narguments;

	jive::fct::lambda_head_op().create_node(lambda->region, 0, nullptr);

	size_t n;
	for (n = 0; n < narguments; n++) {
		jive::gate * gate = jive_graph_create_gate(graph, argument_names[n], *argument_types[n]);
		lambda->arguments[n] = jive_node_gate_output(lambda->region->top, gate);
	}

	return lambda;
}

jive::output *
jive_lambda_end(jive_lambda * self,
	size_t nresults, const jive::base::type * const result_types[], jive::output * const results[])
{
	jive_region * region = self->region;
	jive_graph * graph = region->graph;

	jive_node * leave = jive::fct::lambda_tail_op().create_node(region, 1, &region->top->outputs[0]);

	size_t n;
	for (n = 0; n < nresults; n++) {
		char gate_name[80];
		snprintf(gate_name, sizeof(gate_name), "res_%p_%zd", leave, n);
		jive::gate * gate = jive_graph_create_gate(graph, gate_name, *result_types[n]);
		jive_node_gate_input(leave, gate, results[n]);
	}

	jive_node * anchor = jive_lambda_node_create(region);
	JIVE_DEBUG_ASSERT(anchor->noutputs == 1);

	delete[] self->arguments;
	delete self;

	return anchor->outputs[0];
}

/* lambda inlining */

void
jive_inline_lambda_apply(jive_node * apply_node)
{
	jive_node * lambda_node = apply_node->producer(0);
	
	const jive::fct::lambda_op & op = dynamic_cast<const jive::fct::lambda_op &>(
		lambda_node->operation());
	
	jive_region * function_region = lambda_node->producer(0)->region;
	jive_node * head = function_region->top;
	jive_node * tail = function_region->bottom;
	
	jive::substitution_map substitution;
	for(size_t n = 0; n < op.function_type().narguments(); n++) {
		jive::output * output = jive_node_get_gate_output(head, op.argument_names()[n].c_str());
		substitution.insert(output, apply_node->inputs[n+1]->origin());
	}
	
	jive_region_copy_substitute(function_region,
		apply_node->region, substitution, false, false);
	
	for(size_t n = 0; n < op.function_type().nreturns(); n++) {
		jive::input * input = jive_node_get_gate_input(tail, op.result_names()[n].c_str());
		jive::output * substituted = substitution.lookup(input->origin());
		jive::output * output = apply_node->outputs[n];
		jive_output_replace(output, substituted);
	}
}

/* lambda dead parameters removal */

static bool
lambda_parameter_is_unused(const jive::output * parameter)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::fct::lambda_head_op *>(
		&parameter->node()->operation()));

	return parameter->no_user();
}

static bool
lambda_parameter_is_passthrough(const jive::output * parameter)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::fct::lambda_head_op *>(
		&parameter->node()->operation()));

	jive_node * leave = parameter->node()->region->bottom;
	return parameter->single_user() && (parameter->users.first->node() == leave);
}

static bool
lambda_result_is_passthrough(const jive::input * result)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::fct::lambda_tail_op *>(
		&result->node()->operation()));

	if (dynamic_cast<const jive::fct::lambda_head_op *>(&result->producer()->operation()))
		return lambda_parameter_is_passthrough(result->origin());

	return false;
}

static void
replace_apply_node(const jive_node * apply,
	jive::output * new_fct, const jive_node * old_lambda,
	std::vector<jive::output *> & alive_parameters,
	std::vector<jive::input *> & alive_results)
{
	const jive_node * old_leave = old_lambda->inputs[0]->origin()->node();

	/* collect the arguments for the new apply node */
	std::vector<jive::output *> alive_arguments;
	for (auto parameter : alive_parameters) {
		alive_arguments.push_back(apply->inputs[parameter->index]->origin());
	}

	std::vector<jive::output *> new_apply_results =
		jive_apply_create(new_fct, alive_arguments.size(), &alive_arguments[0]);

	/* replace outputs from old apply node through the outputs from the new one */
	size_t nalive_apply_results = 0;
	for (size_t n = 1; n < old_leave->ninputs; n++) {
		jive::input * result = old_leave->inputs[n];
		if (nalive_apply_results < alive_results.size() && result == alive_results[nalive_apply_results])
			jive_output_replace(apply->outputs[n-1], new_apply_results[nalive_apply_results++]);
		else
			jive_output_replace(apply->outputs[n-1], apply->inputs[result->index()]->origin());
	}
	JIVE_DEBUG_ASSERT(alive_results.size() == nalive_apply_results);
}

static void
replace_all_apply_nodes(jive::output * fct,
	jive::output * new_fct, const jive_node * old_lambda,
	std::vector<jive::output*> alive_parameters,
	std::vector<jive::input*> alive_results)
{
	bool is_lambda = dynamic_cast<const jive::fct::lambda_op *>(&fct->node()->operation());
	bool is_phi_enter = dynamic_cast<const jive::phi_head_op *>(&fct->node()->operation());
	bool is_phi = dynamic_cast<const jive::phi_op *>(&fct->node()->operation());
	JIVE_DEBUG_ASSERT(is_lambda || is_phi_enter || is_phi);

	jive::input * user;
	JIVE_LIST_ITERATE(fct->users, user, output_users_list) {
		if (dynamic_cast<const jive::fct::apply_op *>(&user->node()->operation()))
			replace_apply_node(user->node(), new_fct, old_lambda, alive_parameters, alive_results);

		if (dynamic_cast<const jive::phi_tail_op *>(&user->node()->operation())) {
			jive_node * phi_leave = user->node();

			/* adjust the outer call sides */
			jive_node * phi_node = phi_leave->outputs[0]->users.first->node();
			new_fct = phi_node->outputs[phi_node->noutputs-1];
			replace_all_apply_nodes(phi_node->outputs[user->index()-1], new_fct, old_lambda,
				alive_parameters, alive_results);

			/* adjust the inner call sides */
			jive_node * phi_enter = phi_leave->region->top;
			new_fct = phi_enter->outputs[phi_enter->noutputs-1];
			replace_all_apply_nodes(phi_enter->outputs[user->index()], new_fct, old_lambda,
				alive_parameters, alive_results);

			jive_node_normalize(phi_node);
		}
	}
}


bool
jive_lambda_node_remove_dead_parameters(const jive_node * self)
{
	JIVE_DEBUG_ASSERT(self->noutputs == 1);

	const jive_region * lambda_region = jive_node_anchored_region(self, 0);
	const jive_node * enter = lambda_region->top;
	const jive_node * leave = lambda_region->bottom;
	jive::output * fct = self->outputs[0];

	/* collect liveness information about parameters */
	size_t n;
	size_t nparameters = enter->noutputs-1;
	std::vector<jive::output*> alive_parameters;
	std::vector<const jive::base::type*> alive_parameter_types;
	std::vector<const char*> alive_parameter_names;
	for (n = 1; n < enter->noutputs; n++) {
		jive::output * parameter = enter->outputs[n];

		if (lambda_parameter_is_unused(parameter))
			continue;
		if (lambda_parameter_is_passthrough(parameter))
			continue;

		alive_parameters.push_back(parameter);
		alive_parameter_types.push_back(&parameter->type());
		alive_parameter_names.push_back(parameter->gate->name.c_str());
	}

	/* all parameters are alive, we don't need to do anything */
	if (alive_parameters.size() == nparameters)
		return false;

	/* collect liveness information about results */
	std::vector<jive::input*> alive_results;
	std::vector<const jive::base::type*> alive_result_types;
	for (n = 1; n < leave->ninputs; n++) {
		jive::input * result = leave->inputs[n];

		if (lambda_result_is_passthrough(result))
			continue;

		alive_results.push_back(result);
		alive_result_types.push_back(&result->type());
	}

	/* If the old lambda is embedded within a phi region, extend the phi region with the new lambda */
	bool embedded_in_phi = false;
	jive_node * phi_node = nullptr;
	jive_phi_extension * phi_ext = nullptr;
	if (jive_phi_region_const_cast(lambda_region->parent) != NULL) {
		phi_node = jive_region_get_anchor(lambda_region->parent);

		jive::fct::type fcttype(alive_parameter_types.size(), &alive_parameter_types[0],
			alive_result_types.size(), &alive_result_types[0]);
		const jive::base::type * tmparray0[] = {&fcttype};
		phi_ext = jive_phi_begin_extension(phi_node, 1, tmparray0);
		embedded_in_phi = true;
	}

	/* create new lambda */
	jive::substitution_map map;
	jive_lambda * lambda = jive_lambda_begin(lambda_region->parent, alive_parameter_types.size(),
		&alive_parameter_types[0], &alive_parameter_names[0]);

	for (n = 0; n < alive_parameters.size(); n++)
		map.insert(alive_parameters[n], lambda->arguments[n]);

	jive_region_copy_substitute(lambda_region, lambda->region, map, false, false);

	jive::output * new_results[alive_results.size()];
	for (n = 0; n < alive_results.size(); n++)
		new_results[n] = map.lookup(alive_results[n]->origin());

	jive::output * new_fct = jive_lambda_end(lambda, alive_result_types.size(), &alive_result_types[0],
		new_results);

	/* end the phi extension */
	if (embedded_in_phi) {
		phi_ext->fixvars[0] = new_fct;
		jive_phi_end_extension(phi_ext);
	}

	replace_all_apply_nodes(fct, new_fct, self, alive_parameters, alive_results);

	return true;
}
