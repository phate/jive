/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>

#include <stdio.h>
#include <string.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/phi.h>
#include <jive/vsdg/substitution.h>

/* lambda enter node */

static jive_node *
jive_lambda_enter_node_create(jive_region * region)
{
	JIVE_DEBUG_ASSERT(region->top == NULL && region->bottom == NULL);
	jive_node * node = jive::create_operation_node(jive_op_lambda_enter());
	
	node->class_ = &JIVE_LAMBDA_ENTER_NODE;
	jive_control_type ctl;
	const jive_type * ctl_ptr = &ctl;
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &ctl_ptr);
	((jive_control_output *)node->outputs[0])->set_active(false);
	region->top = node;
	
	return node;
}

static jive_node *
jive_lambda_enter_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	return jive_lambda_enter_node_create(region);
}

const jive_node_class JIVE_LAMBDA_ENTER_NODE = {
	parent : &JIVE_NODE,
	name : "LAMBDA_ENTER",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : nullptr,
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : NULL,
	create : jive_lambda_enter_node_create_, /* override */
};

/* lambda leave node */

static jive_node *
jive_lambda_leave_node_create(jive_output * output)
{
	JIVE_DEBUG_ASSERT(output->node->region->bottom == NULL);
	
	jive_node * node = jive::create_operation_node(jive_op_lambda_leave());
	
	node->class_ = &JIVE_LAMBDA_LEAVE_NODE;
	jive_control_type ctl;
	const jive_type * ctl_ptr = &ctl;
	jive_anchor_type anchor;
	const jive_type * ancptr = &anchor;
	jive_node_init_(node, output->node->region,
		1, &ctl_ptr, &output,
		1, &ancptr);
	output->node->region->bottom = node;
	
	return node;
}

static jive_node *
jive_lambda_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands > 0);
	return jive_lambda_leave_node_create(operands[0]);
}

const jive_node_class JIVE_LAMBDA_LEAVE_NODE = {
	parent : &JIVE_NODE,
	name : "LAMBDA_LEAVE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : nullptr,
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : NULL,
	create : jive_lambda_leave_node_create_, /* override */
};

/* lambda node */

jive_op_lambda::~jive_op_lambda() noexcept
{
}

jive_op_lambda::jive_op_lambda(
	const jive_op_lambda & other)
	: jive_op_lambda(
		other.function_type_,
		other.argument_gates_,
		other.return_gates_)
{
}

jive_op_lambda::jive_op_lambda(
	jive_op_lambda && other)
	: jive_op_lambda(
		std::move(other.function_type_),
		std::move(other.argument_gates_),
		std::move(other.return_gates_))
{
}

jive_op_lambda::jive_op_lambda(
	const jive_function_type & function_type,
	const std::vector<jive_gate *> & argument_gates,
	const std::vector<jive_gate *> & return_gates)
	: function_type_(function_type)
	, argument_gates_(argument_gates)
	, return_gates_(return_gates)
{
}

jive_op_lambda::jive_op_lambda(
	jive_function_type && function_type,
	std::vector<jive_gate *> && argument_gates,
	std::vector<jive_gate *> && return_gates) noexcept
	: function_type_(std::move(function_type))
	, argument_gates_(std::move(argument_gates))
	, return_gates_(std::move(return_gates))
{
}

static void
jive_lambda_node_init_(
	jive_lambda_node * self,
	jive_region * function_region)
{
	jive_region * region = function_region->parent;
	jive_context * context = function_region->graph->context;
	
	jive_anchor_type anchor_type;
	const jive_type * ancptr = &anchor_type;	
	const jive_type * function_type = &self->operation().function_type();
	jive_node_init_(self, region,
		1, &ancptr, &function_region->bottom->outputs[0],
		1, &function_type);
}

static jive_node *
jive_lambda_node_create(jive_region * function_region)
{
	jive_context * context = function_region->graph->context;

	size_t narguments = function_region->top->noutputs - 1;
	size_t nreturns = function_region->bottom->ninputs - 1;
	
	const jive_type * argument_types[narguments];
	std::vector<jive_gate *> argument_gates;
	for (size_t n = 0; n < narguments; n++) {
		argument_types[n] = jive_output_get_type(function_region->top->outputs[n+1]);
		argument_gates.push_back(function_region->top->outputs[n+1]->gate);
	}
	const jive_type * return_types[nreturns];
	std::vector<jive_gate *> return_gates;
	for (size_t n = 0; n < nreturns; n++) {
		return_types[n] = jive_input_get_type(function_region->bottom->inputs[n+1]);
		return_gates.push_back(function_region->bottom->inputs[n+1]->gate);
	}
	
	jive_function_type function_type(
		narguments, argument_types,
		nreturns, return_types);
	
	jive_op_lambda op(std::move(function_type), std::move(argument_gates), std::move(return_gates));
	
	jive_lambda_node * node = new jive_lambda_node(op);
	node->class_ = &JIVE_LAMBDA_NODE;
	jive_lambda_node_init_(node, function_region);
	return node;
}

static void
jive_lambda_node_fini_(jive_node * self_)
{
	jive_node_fini_(self_);
}

static jive_node *
jive_lambda_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands > 0);
	return jive_lambda_node_create(operands[0]->node->region);
}

static bool
jive_lambda_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_op_lambda * first = &((const jive_lambda_node *)self)->operation();
	const jive_op_lambda * second = (const jive_op_lambda *) attrs;

	return
		jive_type_equals(&first->function_type(), &second->function_type()) &&
		first->argument_gates() == second->argument_gates() &&
		first->return_gates() == second->return_gates();
}

const jive_node_class JIVE_LAMBDA_NODE = {
	parent : &JIVE_NODE,
	name : "LAMBDA",
	fini : jive_lambda_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : nullptr, /* inherit */
	match_attrs : jive_lambda_node_match_attrs_, /* override */
	check_operands : NULL,
	create : jive_lambda_node_create_, /* override */
};

bool
jive_lambda_is_self_recursive(const jive_lambda_node * self_)
{
	JIVE_DEBUG_ASSERT(self_->noutputs == 1);

	const jive_node * self = self_;
	const jive_region * lambda_region = jive_lambda_node_get_region(self_);

	if (jive_phi_region_const_cast(self->region) == NULL)
		return false;

	/* find index of lambda output in the phi leave node */
	jive_input * user;
	size_t index = self->region->top->noutputs;
	JIVE_LIST_ITERATE(self->outputs[0]->users, user, output_users_list) {
		if (jive_node_isinstance(user->node, &JIVE_PHI_LEAVE_NODE)) {
			index = user->index;
			break;
		}
	}
	JIVE_DEBUG_ASSERT(index != self->region->top->noutputs);

	/* the lambda is self recursive if the function input of an apply node in the lambda region
		originates from the same index in the phi enter node */
	jive_region_hull_entry * entry;
	JIVE_LIST_ITERATE(lambda_region->hull, entry, input_hull_list) {
		if (!jive_node_isinstance(entry->input->node, &JIVE_APPLY_NODE))
			continue;
		if (!jive_node_isinstance(entry->input->origin->node, &JIVE_PHI_ENTER_NODE))
			continue;

		if (entry->input->origin->index == index)
			return true;
	}

	return false;
}

/* lambda instantiation */

typedef struct jive_lambda_build_state jive_lambda_build_state;
struct jive_lambda_build_state {
	jive_floating_region floating;
};

struct jive_lambda *
jive_lambda_begin(struct jive_graph * graph,
	size_t narguments, const jive_type * const argument_types[], const char * const argument_names[])
{
	jive_context * context = graph->context;

	jive_lambda * lambda = jive_context_malloc(context, sizeof(*lambda));
	jive_lambda_build_state * state;
	state = jive_context_malloc(graph->context, sizeof(*state));
	state->floating = jive_floating_region_create(graph);
	lambda->region = state->floating.region;
	lambda->arguments = jive_context_malloc(graph->context, sizeof(*lambda->arguments) * narguments);
	lambda->narguments = narguments;

	jive_lambda_enter_node_create(lambda->region);

	size_t n;
	for (n = 0; n < narguments; n++) {
		jive_gate * gate = jive_type_create_gate(argument_types[n], graph, argument_names[n]);
		lambda->arguments[n] = jive_node_gate_output(lambda->region->top, gate);
	}

	lambda->internal_state = state;

	return lambda;
}

jive_output *
jive_lambda_end(jive_lambda * self,
	size_t nresults, const jive_type * const result_types[], jive_output * const results[])
{
	jive_lambda_build_state * state = self->internal_state;
	jive_region * region = self->region;
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;

	jive_node * leave = jive_lambda_leave_node_create(region->top->outputs[0]);

	size_t n;
	for (n = 0; n < nresults; n++) {
		char gate_name[80];
		snprintf(gate_name, sizeof(gate_name), "res_%p_%zd", leave, n);
		jive_gate * gate = jive_type_create_gate(result_types[n], graph, gate_name);
		jive_node_gate_input(leave, gate, results[n]);
	}

	jive_floating_region_settle(state->floating);

	jive_node * anchor = jive_lambda_node_create(region);
	JIVE_DEBUG_ASSERT(anchor->noutputs == 1);

	jive_context_free(context, self->arguments);
	jive_context_free(context, state);
	jive_context_free(context, self);

	return anchor->outputs[0];
}

/* lambda inlining */

void
jive_inline_lambda_apply(jive_node * apply_node)
{
	jive_lambda_node * lambda_node = jive_lambda_node_cast(apply_node->inputs[0]->origin->node);
	JIVE_DEBUG_ASSERT(lambda_node);
	if (!lambda_node)
		return;
	
	jive_region * function_region = lambda_node->inputs[0]->origin->node->region;
	jive_node * head = function_region->top;
	jive_node * tail = function_region->bottom;
	
	jive_substitution_map * substitution = jive_substitution_map_create(apply_node->graph->context);
	
	for(size_t n = 0; n < lambda_node->operation().function_type().narguments(); n++) {
		jive_gate * gate = lambda_node->operation().argument_gates()[n];
		jive_output * output = jive_node_get_gate_output(head, gate);
		jive_substitution_map_add_output(substitution, output, apply_node->inputs[n+1]->origin);
	}
	
	jive_region_copy_substitute(function_region,
		apply_node->region, substitution, false, false);
	
	for(size_t n = 0; n < lambda_node->operation().function_type().nreturns(); n++) {
		jive_gate * gate = lambda_node->operation().return_gates()[n];
		jive_input * input = jive_node_get_gate_input(tail, gate);
		jive_output * substituted = jive_substitution_map_lookup_output(substitution, input->origin);
		jive_output * output = apply_node->outputs[n];
		jive_output_replace(output, substituted);
	}
	
	jive_substitution_map_destroy(substitution);
}

/* lambda dead parameters removal */

static bool
lambda_parameter_is_unused(const struct jive_output * parameter)
{
	JIVE_DEBUG_ASSERT(jive_node_isinstance(parameter->node, &JIVE_LAMBDA_ENTER_NODE));

	return jive_output_has_no_user(parameter);
}

static bool
lambda_parameter_is_passthrough(const struct jive_output * parameter)
{
	JIVE_DEBUG_ASSERT(jive_node_isinstance(parameter->node, &JIVE_LAMBDA_ENTER_NODE));

	jive_node * leave = parameter->node->region->bottom;
	return jive_output_has_single_user(parameter) && (parameter->users.first->node == leave);
}

static bool
lambda_result_is_passthrough(const struct jive_input * result)
{
	JIVE_DEBUG_ASSERT(jive_node_isinstance(result->node, &JIVE_LAMBDA_LEAVE_NODE));

	if (jive_node_isinstance(result->origin->node, &JIVE_LAMBDA_ENTER_NODE))
		return lambda_parameter_is_passthrough(result->origin);

	return false;
}

static void
replace_apply_node(const jive_apply_node * apply_,
	jive_output * new_fct, const jive_lambda_node * old_lambda,
	size_t nalive_parameters, jive_output * alive_parameters[],
	size_t nalive_results, jive_input * alive_results[])
{
	const struct jive_node * apply = apply_;
	const jive_node * old_leave = jive_lambda_node_get_leave_node(old_lambda);

	/* collect the arguments for the new apply node */
	size_t n;
	jive_output * alive_arguments[nalive_parameters];
	for (n = 0; n < nalive_parameters; n++) {
		size_t index = alive_parameters[n]->index;
		alive_arguments[n] = apply->inputs[index]->origin;
	}

	jive_output * new_apply_results[nalive_results];
	jive_apply_create(new_fct, nalive_parameters, alive_arguments, new_apply_results);

	/* replace outputs from old apply node through the outputs from the new one */
	size_t nalive_apply_results = 0;
	for (n = 1; n < old_leave->ninputs; n++) {
		jive_input * result = old_leave->inputs[n];
		if (result == alive_results[nalive_apply_results])
			jive_output_replace(apply->outputs[n-1], new_apply_results[nalive_apply_results++]);
		else
			jive_output_replace(apply->outputs[n-1], apply->inputs[result->index]->origin);
	}
	JIVE_DEBUG_ASSERT(nalive_results == nalive_apply_results);
}

static void
replace_all_apply_nodes(jive_output * fct,
	jive_output * new_fct, const jive_lambda_node * old_lambda,
	size_t nalive_parameters, jive_output * alive_parameters[],
	size_t nalive_results, jive_input * alive_results[])
{
	bool is_lambda = jive_node_isinstance(fct->node, &JIVE_LAMBDA_NODE);
	bool is_phi_enter = jive_node_isinstance(fct->node, &JIVE_PHI_ENTER_NODE);
	bool is_phi = jive_node_isinstance(fct->node, &JIVE_PHI_NODE);
	JIVE_DEBUG_ASSERT(is_lambda || is_phi_enter || is_phi);

	jive_input * user;
	JIVE_LIST_ITERATE(fct->users, user, output_users_list) {
		const jive_apply_node * apply = jive_apply_node_const_cast(user->node);
		if (apply != NULL)
			replace_apply_node(apply, new_fct, old_lambda,
				nalive_parameters, alive_parameters, nalive_results, alive_results);

		if (jive_node_isinstance(user->node, &JIVE_PHI_LEAVE_NODE)) {
			jive_node * phi_leave = user->node;

			/* adjust the outer call sides */
			jive_node * phi_node = phi_leave->outputs[0]->users.first->node;
			new_fct = phi_node->outputs[phi_node->noutputs-1];
			replace_all_apply_nodes(phi_node->outputs[user->index-1], new_fct,
				old_lambda, nalive_parameters, alive_parameters, nalive_results, alive_results);

			/* adjust the inner call sides */
			jive_node * phi_enter = phi_leave->region->top;
			new_fct = phi_enter->outputs[phi_enter->noutputs-1];
			replace_all_apply_nodes(phi_enter->outputs[user->index], new_fct, old_lambda,
				nalive_parameters, alive_parameters, nalive_results, alive_results);

			jive_node_normalize(phi_node);
		}
	}
}


bool
jive_lambda_node_remove_dead_parameters(const jive_lambda_node * self)
{
	JIVE_DEBUG_ASSERT(self->noutputs == 1);

	jive_graph * graph = self->region->graph;
	jive_context * context = graph->context;
	const jive_node * enter = jive_lambda_node_get_enter_node(self);
	const jive_node * leave = jive_lambda_node_get_leave_node(self);
	const jive_region * lambda_region = enter->region;
	jive_output * fct = self->outputs[0];

	/* collect liveness information about parameters */
	size_t n;
	size_t nalive_parameters = 0;
	size_t nparameters = enter->noutputs-1;
	jive_output * alive_parameters[nparameters];
	const jive_type * alive_parameter_types[nparameters];
	const char * alive_parameter_names[nparameters];
	for (n = 1; n < enter->noutputs; n++) {
		jive_output * parameter = enter->outputs[n];

		if (lambda_parameter_is_unused(parameter))
			continue;
		if (lambda_parameter_is_passthrough(parameter))
			continue;

		alive_parameters[nalive_parameters] = parameter;
		alive_parameter_types[nalive_parameters] = jive_output_get_type(parameter);
		alive_parameter_names[nalive_parameters++] = parameter->gate->name;
	}

	/* all parameters are alive, we don't need to do anything */
	if (nalive_parameters == nparameters)
		return false;

	/* collect liveness information about results */
	size_t nalive_results = 0;
	size_t nresults = leave->ninputs-1;
	jive_input * alive_results[nresults];
	const jive_type * alive_result_types[nresults];
	for (n = 1; n < leave->ninputs; n++) {
		jive_input * result = leave->inputs[n];

		if (lambda_result_is_passthrough(result))
			continue;

		alive_results[nalive_results] = result;
		alive_result_types[nalive_results++] = jive_input_get_type(result);
	}

	/* If the old lambda is embedded within a phi region, extend the phi region with the new lambda */
	bool embedded_in_phi = false;
	jive_phi_node * phi_node = NULL;
	jive_phi_extension * phi_ext = NULL;
	if (jive_phi_region_const_cast(lambda_region->parent) != NULL) {
		phi_node = jive_phi_node_cast(jive_region_get_anchor(lambda_region->parent));

		jive_function_type fcttype(nalive_parameters, alive_parameter_types, nalive_results,
			alive_result_types);
		const jive_type * tmparray0[] = {&fcttype};
		phi_ext = jive_phi_begin_extension(phi_node, 1, tmparray0);
		embedded_in_phi = true;
	}

	/* create new lambda */
	jive_substitution_map * map = jive_substitution_map_create(context);
	jive_lambda * lambda = jive_lambda_begin(graph, nalive_parameters, alive_parameter_types,
		alive_parameter_names);

	for (n = 0; n < nalive_parameters; n++)
		jive_substitution_map_add_output(map, alive_parameters[n], lambda->arguments[n]);

	jive_region_copy_substitute(lambda_region, lambda->region, map, false, false);

	jive_output * new_results[nalive_results];
	for (n = 0; n < nalive_results; n++) {
		new_results[n] = jive_substitution_map_lookup_output(map, alive_results[n]->origin);
		JIVE_DEBUG_ASSERT(new_results[n] != NULL);
	}

	jive_output * new_fct = jive_lambda_end(lambda, nalive_results, alive_result_types, new_results);
	jive_substitution_map_destroy(map);

	/* end the phi extension */
	if (embedded_in_phi) {
		phi_ext->fixvars[0] = new_fct;
		jive_phi_end_extension(phi_ext);
	}

	replace_all_apply_nodes(fct, new_fct, (const jive_lambda_node *)self,
		nalive_parameters, alive_parameters, nalive_results, alive_results);

	return true;
}
